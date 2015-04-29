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
#include "getTime.h"

using namespace std;

StarCamera starCam; /*!< Global StarCamera object performs the image aquisition (from file or from camera) and
                    extract bright spots as possible stars for identification */

StarIdentifier starId; /*!< Global StarIdentifier which performs the identification process of the previously
                            extracted star spots (from StarCamera) */

bool printStats; /*!< TODO */

// Parse the command line arguments
// Define possible arguments
TCLAP::CmdLine cmd("Program for attitude estimation from star images",' ', "0.1");

TCLAP::ValueArg<float> epsilon("e", "epsilon", "The allowed tolerance for the feature (in degrees)", false, 0.1, "float");
TCLAP::ValueArg<string> test("", "test", "Run test specified test (all other input will be ignored):\n -camera: Grab a frame from camera and display it on screen", false, string(), "string");
TCLAP::ValueArg<unsigned> area("a", "area", "The minimum area (in pixel) for a spot to be considered for identification", false, 16, "unsigned int");
TCLAP::ValueArg<unsigned> threshold("t", "threshold", "Threshold under which pixels are set to 0", false, 64, "unsigned int");
TCLAP::ValueArg<string> calibrationFile("", "calibration", "Set the calibration file for the camera manually", false, "/home/jan/workspace/usu/starcamera/bin/aptina_12_5mm-calib.txt", "filename");
TCLAP::ValueArg<string> initFile("", "init", "Set the file for initialization of the Aptina camera", false, string(), "filename");
TCLAP::ValueArg<string> dbFile("", "db", "Set the file containing the featurelist in the SQLite database format", false, string(), "filename");
TCLAP::ValueArg<string> kVectorFile("", "kvector", "Set the for loading kVector information", false, "/home/jan/workspace/usu/starcamera/bin/kVectorInput.txt", "filename");

TCLAP::SwitchArg stats("s", "stats", "Print statistics (number of spots, number of identified spots, ratio");
TCLAP::SwitchArg raw("", "raw", "Input files are raw Bayer-12-files");
TCLAP::UnlabeledMultiArg<string> files("fileNames", "List of filenames of the raw-image files", false, "file1");


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
 \brief Printing function for Spot structure

 \param os
 \param spot
 \return std::ostream &operator
*/
std::ostream & operator << (std::ostream & os, const Spot& spot)
{
    os << spot.center.x << "\t" << spot.center.y << "\t" << spot.area;
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
        os << spots[i] << "\t" << starID[i] << endl;
    }
}


/*!
 \brief Generates data for the different centroiding functions for each file in files



*/
void centroidingComparison()
{
    vector<string> fileNames = files.getValue();
    for (vector<string>::const_iterator file = fileNames.begin(); file!=fileNames.end(); ++file)
    {
        // for each file extract spots with all methods and print data to files, together with runtime
        if(raw.getValue())
            starCam.getImageFromRaw(*file);
        else
            starCam.getImageFromFile(*file);

        double endTime, startTime;
        vector<double> runtimes;
        vector<vector<Spot> > spotLists;

        // call extract spots with every extraction method and
        // copy results to spotLists and save measured
        // runtime in runtimes
        startTime = getRealTime();
        starCam.extractSpots(StarCamera::ContoursGeometric);
        endTime = getRealTime();
        runtimes.push_back(endTime - startTime);
        spotLists.push_back(starCam.getSpots());

        startTime = getRealTime();
        starCam.extractSpots(StarCamera::ContoursWeighted);
        endTime = getRealTime();
        runtimes.push_back(endTime - startTime);
        spotLists.push_back(starCam.getSpots());

        startTime = getRealTime();
        starCam.extractSpots(StarCamera::ContoursWeightedBoundingBox);
        endTime = getRealTime();
        runtimes.push_back(endTime - startTime);
        spotLists.push_back(starCam.getSpots());

        startTime = getRealTime();
        starCam.extractSpots(StarCamera::ConnectedComponentsGeometric);
        endTime = getRealTime();
        runtimes.push_back(endTime - startTime);
        spotLists.push_back(starCam.getSpots());

        startTime = getRealTime();
        starCam.extractSpots(StarCamera::ConnectedComponentsWeighted);
        endTime = getRealTime();
        runtimes.push_back(endTime - startTime);
        spotLists.push_back(starCam.getSpots());

        /* print the data in the following form (for m>n):
         * File: <filename>
         * <list of runtimes of all methods>
         * <spot1 of method1><spot1 of method2><...>
         * [...]
         * <spot n of method1>spot n of method2><...>
         * <-1><spot n+1 of method2><...>
         * [...]
         * <-1><spot m of method2><...>
         */

        // print a file identifier
        unsigned pos = file->find_last_of("/\\");
        cout << "File: " << file->substr(pos+1, file->size()-5-pos) << endl;

        cout << runtimes[0] << "\t" << runtimes[1] << "\t"
             << runtimes[2] << "\t" << runtimes[3] << "\t"
             << runtimes[4] << endl;

        vector<vector<Spot>::const_iterator > it;
        it.push_back(spotLists[0].begin() );
        it.push_back(spotLists[1].begin() );
        it.push_back(spotLists[2].begin() );
        it.push_back(spotLists[3].begin() );
        it.push_back(spotLists[4].begin() );

        while( (it[0]!=spotLists[0].end()) || (it[1]!=spotLists[1].end()) ||
               (it[2]!=spotLists[2].end()) || (it[3]!=spotLists[3].end()) ||
               (it[4]!=spotLists[4].end()))
        {

            for(unsigned i=0; i<5; ++i)
            {
                if (it[i] != spotLists[i].end() )
                {
                    cout << *(it[i]) << "\t";
                    it[i]++;
                }
                else
                {
                    cout << -1 << "\t" << -1 << "\t" << -1 << "\t";
                }
            }
            cout << endl;
        }
    }

}

/*!
 \brief Generates data for the different identification methods

*/
void identificationComparison()
{
    const float eps = epsilon.getValue();
    vector<string> fileNames = files.getValue();
    for (vector<string>::const_iterator file = fileNames.begin(); file!=fileNames.end(); ++file)
    {
        if(raw.getValue())
            starCam.getImageFromRaw(*file);
        else
            starCam.getImageFromFile(*file);
        starCam.extractSpots();
        starCam.calculateSpotVectors();

        starId.setFeatureListDB(dbFile.getValue());
        starId.openDb();
        starId.loadFeatureListKVector(kVectorFile.getValue());

        double endTime, startTime;
        vector<double> runtimes;
        vector<vector<int> > idLists;

        startTime = getRealTime();
        idLists.push_back(starId.identifyStars(starCam.getSpotVectors(), eps, StarIdentifier::TwoStar) );
        endTime = getRealTime();
        runtimes.push_back(endTime-startTime);

        startTime = getRealTime();
        idLists.push_back(starId.identifyStars(starCam.getSpotVectors(), eps, StarIdentifier::PyramidSQL) );
        endTime = getRealTime();
        runtimes.push_back(endTime-startTime);

        startTime = getRealTime();
        idLists.push_back(starId.identifyStars(starCam.getSpotVectors(), eps, StarIdentifier::PyramidKVector) );
        endTime = getRealTime();
        runtimes.push_back(endTime-startTime);

        /* print the data in the following form (for m>n):
         * File: <filename>
         * <list of runtimes of all methods>
         * <spot1><id for spot1 of method1><id for spot1 of method2><...>
         * [...]
         * <spotn><id for spotn of methodn><id for spotn of methodn><...>
         */

        // print a file identifier
        unsigned pos = file->find_last_of("/\\");
        cout << "File: " << file->substr(pos+1, file->size()-5-pos) << endl;

        cout << runtimes[0] << "\t" << runtimes[1] << "\t"
             << runtimes[2] << endl;

        const vector<Spot> & spotList = starCam.getSpots();
        for(unsigned i=0; i<spotList.size(); ++i)
        {
            cout << spotList[i] << "\t";
            cout << idLists[0][i] << "\t" << idLists[1][i] << "\t" << idLists[2][i] << endl;
        }

    }
}


/*!
 \brief Quick and dirty function atm

 \param eps
*/
void identifyStars(float eps)
{
    starCam.extractSpots(StarCamera::ConnectedComponentsWeighted);
    try
    {
        starCam.calculateSpotVectors();

        starId.setFeatureListDB("/home/jan/workspace/usu/starcamera/bin/featureList2.db");
        starId.openDb();

        starId.loadFeatureListKVector(kVectorFile.getValue());


        std::vector<int> idStars = starId.identifyStars(starCam.getSpotVectors(),eps, StarIdentifier::PyramidKVector);

        if(printStats)
            outputStats(cout, idStars, starCam.getSpots());
        else
            cout << idStars;

        cout << endl;
    }
    catch (...)
    {
        cout << "No spots" << endl;
    }
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
    /* Avoids memory swapping for this program */
    mlockall(MCL_CURRENT|MCL_FUTURE);

    try
    {
        // Register arguments to parser
        cmd.add(epsilon);
        cmd.add(test);
        cmd.add(area);
        cmd.add(threshold);
        cmd.add(calibrationFile);
        cmd.add(initFile);
        cmd.add(dbFile);
        cmd.add(kVectorFile);
        cmd.add(stats);
        cmd.add(files);
        cmd.add(raw);

        cmd.parse(argc, argv);

        // Get parsed arguments
        float eps = epsilon.getValue();
        starCam.setMinArea(area.getValue() );
        starCam.setThreshold(threshold.getValue());
        starCam.loadCalibration(calibrationFile.getValue());
        printStats = stats.getValue();

        // check if in test mode
        string testRoutine = test.getValue();
        if(!testRoutine.empty())
        {
            if (testRoutine == "centroiding")
            {
                centroidingComparison();
            }
            if (testRoutine == "identification")
            {
                identificationComparison();
            }
            return 0;
        }

        // get the filenames
        std::vector<string> fileNames = files.getValue();

        // for every filename identify the stars and print the results
        for(std::vector<string>::const_iterator file = fileNames.begin(); file != fileNames.end(); ++file)
        {
            if(raw.getValue())
                starCam.getImageFromRaw(*file);
            else
                starCam.getImageFromFile(*file);

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

