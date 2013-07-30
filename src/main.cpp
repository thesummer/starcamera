#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <sys/time.h>
#include <sys/mman.h>
#include <sched.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "tclap/CmdLine.h"
#include "starcamera.h"
#include "starid.h"

using namespace std;

StarCamera starCam; /*!< Global StarCamera object performs the image aquisition (from file or from camera) and
                    extract bright spots as possible stars for identification */

StarIdentifier starId; /*!< Global StarIdentifier which performs the identification process of the previously
                            extracted star spots (from StarCamera) */

bool printStats; /*!< TODO */

/*!
 \brief Function to substract timeval structures

    result = x-y

 \param result
 \param x
 \param y
 \return int
*/
int timeval_subtract (struct timeval * result, struct timeval * x, struct timeval * y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
          tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}

/*!
 \brief Printing function for vector<int>

 \param os
 \param vector
 \return std::ostream &operator
*/
std::ostream & operator << (std::ostream & os, const std::vector<int>& vector)
{
    for(unsigned int i = 0; i<vector.size(); ++i)
    {
        cout << i << '\t' << vector[i] << endl;
    }
    return os;
}

/*!
 \brief Printing function for information of the star-id process

 Prints:
    Coordinates of the center of the extracted star Spot
    The area of the extracted star Spot
    The hip-ID determined for the star Spot (-1 if no identification possible)

 \param os
 \param starID
 \param spots
*/
void outputStats(std::ostream & os, const std::vector<int>& starID, const std::vector<Spot>& spots)
{
    if (starID.size() != spots.size())
        throw std::runtime_error("List of identified spot must have same size as list of extracted spots");
    for(unsigned int i = 0; i<spots.size(); ++i)
    {
        os << spots[i].center.x << "\t" << spots[i].center.y << "\t";
        os << spots[i].area << "\t" << starID[i] << endl;
    }
}

/*!
 \brief Quick and dirty function atm

 \param eps
*/
void identifyStars(float eps)
{
    starCam.extractSpots();
    starCam.calculateSpotVectors();

    //    starId.setFeatureListDB("/home/jan/workspace/usu/starcamera/bin/featureList2.db");
    //    starId.openDb();

    starId.loadFeatureListKVector("/home/jan/workspace/usu/starcamera/bin/kVectorInput.txt");

    //    starId.identifyPyramidMethod(starCam.getSpotVectors(), eps);

    std::vector<int> idStars = starId.identifyStars(starCam.getSpotVectors(),eps);

    if(printStats)
        outputStats(cout, idStars, starCam.getSpots());
    else
        cout << idStars;

    cout << endl;
}

/*!
 \brief Function to run a live identification using a picture from the Aptina

 \param eps
*/
void liveIdentification(float eps)
{
    static unsigned counter = 0;

    cout << "File: " << counter << endl;
    starCam.getImage();
    identifyStars(eps);

    counter++;
}

/*!
 \brief Main function

 Handles the commandline arguments, sets the options accordingly and
 runs the program.

 \param argc
 \param argv
 \return int
*/
int main(int argc, char **argv)
{
//    struct sched_param param;

//    param.__sched_priority = 50;

//    if( sched_setscheduler( 0, SCHED_FIFO, &param ) == -1 )
//    {
//        perror("sched_setscheduler");
//    }

    try
    {

        // Parse the command line arguments
        // Define possible arguments
        TCLAP::CmdLine cmd("Program for attitude estimation from star images",' ', "0.1");

        TCLAP::ValueArg<float> epsilon("e", "epsilon", "The allowed tolerance for the feature (in degrees)", false, 0.1, "float");
        TCLAP::ValueArg<string> test("t", "test", "Run test specified test (all other input will be ignored):\n -camera: Grab a frame from camera and display it on screen", false, string(), "string");
        TCLAP::ValueArg<unsigned> area("a", "area", "The minimum area (in pixel) for a spot to be considered for identification", false, 16, "unsigned int");
        TCLAP::ValueArg<string> calibrationFile("", "calibration", "Set the calibration file for the camera manually", false, "/home/jan/workspace/usu/starcamera/bin/aptina_12_5mm-calib.txt", "filename");
        TCLAP::ValueArg<string> initFile("", "init", "Set the file for initialization of the Aptina camera", false, string(), "filename");

        TCLAP::SwitchArg stats("s", "stats", "Print statistics (number of spots, number of identified spots, ratio");
        TCLAP::SwitchArg useCamera("c", "camera", "Use the connected Aptina camera as input (input files will be ignored)");
        TCLAP::UnlabeledMultiArg<string> files("fileNames", "List of filenames of the raw-image files", false, "file1");

        // Register arguments to parser
        cmd.add(epsilon);
        cmd.add(test);
        cmd.add(area);
        cmd.add(calibrationFile);
        cmd.add(initFile);
        cmd.add(stats);
        cmd.add(useCamera);
        cmd.add(files);

        cmd.parse(argc, argv);

        // check if in test mode
        string testRoutine = test.getValue();
        if(!testRoutine.empty())
        {
            if (testRoutine == "camera")
            {
                if (initFile.getValue().empty())
                    starCam.initializeCamera(NULL);
                else
                    starCam.initializeCamera(initFile.getValue().c_str());

                starCam.cameraTest();
                return 0;
            }
        }

        // Get parsed arguments
        float eps = epsilon.getValue();
        starCam.setMinArea(area.getValue() );
        starCam.loadCalibration(calibrationFile.getValue().c_str());
        printStats = stats.getValue();

        // check if camera is to be used
        if(useCamera.getValue() )
        {
            if (initFile.getValue().empty())
                starCam.initializeCamera(NULL);
            else
                starCam.initializeCamera(initFile.getValue().c_str());
            liveIdentification(eps); // add options for multiple pictures and delay?

            return 0;
        }
        // else use saved raw images to identifiy stars


        // get the filenames
        std::vector<string> fileNames = files.getValue();
        /* Avoids memory swapping for this program */
        //    mlockall(MCL_CURRENT|MCL_FUTURE);
        //    starCam.setMinRadius(3.0f);

        // for every filename identify the stars and print the results
        for(std::vector<string>::const_iterator file = fileNames.begin(); file != fileNames.end(); ++file)
        {
            starCam.getImageFromFile(file->c_str());

            // print a file identifier
            unsigned pos = file->find_last_of("/\\");
            cout << "File: " << file->substr(pos+1, file->size()-5-pos) << endl;

            identifyStars(eps);
        }

    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
        return 1;
    }

    return 0;
}

