#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/utility.hpp>
#include <unistd.h>

#include <stdexcept>
#include <string>
#include <iostream>
using std::cout;
using std::endl;

#include "starcamera.h"

const float pi = 3.14159265358979323846;

StarCamera::StarCamera()
    :mThreshold(64)
{

}

void StarCamera::initializeCamera(const std::string initFile)
{
    mCamera.initialize(initFile);
}

void StarCamera::getImage()
{
    unsigned height = mCamera.getHeight();
    unsigned width  = mCamera.getWidth();
    mFrame.create(height, width);

    // get Image Data
    uint8_t * tmp = 0;
    // copy image data into frame, thereby changing from 12-bit to 8-bit

    while (! mCamera.grabFrame(&tmp) )
        usleep(10000);

    // iterate in a single for loop
    unsigned length = height * width;

    // pointer to the raw image
    uint16_t * imgBuf = (uint16_t *) tmp;
    // pointer to the data of the mFrame object
    uint8_t  * pFrame = mFrame.data;
    // pointer to the end of the allocated memory
    uint8_t  * pFrameEnd = pFrame + length;

    for(; pFrame != pFrameEnd; ++imgBuf, ++pFrame)
    {
        // calculate the 8-bit value (divide by 16 or shift by 4) for each pixel from the
        // raw image and assign it to the corresponding pixel in mFrame
        *pFrame =  (*imgBuf) >> 4;
    }

}

void StarCamera::getImageFromFile(const std::string filename, const unsigned rows, const unsigned cols)
{
    // open image file
    std::fstream file;
    file.open(filename, std::ios_base::in | std::ios_base::binary);
    if(!file.is_open())
    {
        throw std::runtime_error(std::string("Failed to open image file: ") + filename);
    }

    // resize matrix structure for image data if necessary
    mFrame.create(rows, cols);

    // read image data and transform from 12 to 8 bit
    for (unsigned i = rows-1; i>0; --i)
    {
        for(unsigned j = 0; j<cols; ++j)
        {
            u_int16_t temp;
            file.read((char*) &temp, sizeof(u_int16_t) );

            mFrame(i,j) = temp/16;
        }
    }

    file.close();

    //Usefull?
    mThreshed.release();

    mLabels.create(mFrame.size());

}

unsigned StarCamera::extractSpots(CentroidingMethod method)
{
    mSpots.clear();

    if(!mFrame.data)
    {
        throw std::runtime_error("ExtractSpots: No frame loaded");
    }

    // Threshold the image: set all pixels lower than mThreshold to 0
    cv::threshold(mFrame, mThreshed, mThreshold, 0, cv::THRESH_TOZERO);

    switch (method)
    {
    case ContoursGeometric:
    case ContoursWeighted:
    case ContoursWeightedBoundingBox:
        return CentroidingContours(method);
        break;

    case ConnectedComponentsGeometric:
        return CentroidingConnectedComponentsGeometric();
        break;

    case ConnectedComponentsWeighted:
        return CentroidingConnectedComponentsWeighted();
        break;
    }

    // should never reach this point
    return 0;
}

void StarCamera::calculateSpotVectors()
{
    bool zeroNorm = !(mDistortionCoeffi.norm() != 0.0f);

    // Clear SpotVectors
    mSpotVectors.clear();

    std::vector<Spot>::iterator pSpot;
    for(pSpot=mSpots.begin(); pSpot != mSpots.end(); ++pSpot)
    {
        // Substract principal point and divide by the focal length

        Eigen::Vector2f Xd(pSpot->center.x, pSpot->center.y);
        Xd = (Xd - mPrincipalPoint).array() / mFocalLength.array();

        // Undo skew
        Xd(0) = Xd(0) - mPixelSkew * Xd(1);

        Eigen::Vector3f spotVec;
        if(!zeroNorm)  // Use epsilon environment?
        {
            Xd = undistortRadialTangential(Xd);
        }
        spotVec << Xd(0), Xd(1), 1.0f;
        spotVec.normalize();
        mSpotVectors.push_back(spotVec);
    }
}

void StarCamera::loadCalibration(const std::string filename)
{
    std::fstream file;
    file.open(filename, std::ios_base::in);
    if(!file.is_open())
    {
        /// TODO throw error
    }

    file >> mPrincipalPoint(0);
    file >> mPrincipalPoint(1);

    file >> mPixelSkew;

    for(int i=0; i<5; ++i)
    {
        file >> mDistortionCoeffi(i);
    }

    file >> mFocalLength(0);
    file >> mFocalLength(1);

    file.close();
}

void StarCamera::cameraTest()
{
    unsigned height = mCamera.getHeight();
    unsigned width = mCamera.getWidth();
    mFrame.create(height, width);

    // get Image Data
    uint8_t * tmp = 0;
    char filename[15] = "pic0.png";
    char rawname [15] = "pic0.raw";

    for (int n =0; n<10; ++n)
    {     
        while (! mCamera.grabFrame(&tmp) )
            usleep(10000);

        uint16_t * imgBuf = (uint16_t *) tmp;
        uint16_t * row = imgBuf;
        for(unsigned i=0; i<height; ++i, row+= width)
        {
            uint16_t * col = row;
            for(unsigned j=0; j<width; ++j, ++col)
            {
                mFrame(i,j) = (*col) / 16;
            }
        }

        cv::imshow("Hallo", mFrame);
        cv::waitKey();
        filename[3] = '0' + n;
        cout << filename << endl;
        cv::imwrite(filename, mFrame);
        rawname[3] = '0' + n;
        std::ofstream rawFile;
        rawFile.open(rawname, std::ios_base::binary);
        rawFile.write((const char*) tmp, height*width*sizeof(uint16_t));
        rawFile.close();
    }
}

unsigned StarCamera::CentroidingContours(CentroidingMethod method)
{
    // vector which stores the point belonging to each contour
    std::vector<Contour_t> contours;
    // Find contours in the threshed image
    cv::findContours(mThreshed, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

    // Find matching contours/spots
    std::vector <std::vector<cv::Point> >::iterator it;
    for (it = contours.begin(); it != contours.end(); ++it)
    {
        cv::Point2f center;
        float radius;
        const float minRadius = sqrt(mMinArea / M_PI);

        // find the circle for each contour
        cv::minEnclosingCircle(*it, center, radius);

        // Save the spot if it is large enough
        unsigned area;
        if(radius > minRadius)
        {
            switch(method)
            {
            case ContoursGeometric:
                mSpots.push_back(Spot(center, (unsigned) (pi * radius*radius)+1 ) );
                break;
            case ContoursWeighted:
                // get the area of the current contour
                area = (unsigned) cv::contourArea(*it);
                computeWeightedCentroid(*it, center);
                mSpots.push_back(Spot(center, area));
                break;
            case ContoursWeightedBoundingBox:
                computeWeightedCentroidBoundingRect(*it, center, area);
                mSpots.push_back(Spot(center, area) );
                break;
            }
        }
    }

    return mSpots.size();
}

unsigned StarCamera::CentroidingConnectedComponentsGeometric()
{
    cv::Mat stats;
    cv::Mat centroids;
    int nLabels = cv::connectedComponentsWithStats(mThreshed, mLabels, stats, centroids, 8, CV_16U);

    for(int i=1; i<nLabels; ++i)
    {
        const int indexArea = 4;
        int area = stats.at<int>(i, indexArea);
        if(area > 16)
        {
            mSpots.push_back(Spot(centroids.at<cv::Point2d>(i),area));
        }
    }

    return mSpots.size();
}

unsigned StarCamera::CentroidingConnectedComponentsWeighted()
{
    cv::Mat stats;
    /* Use altered function which calculates desired stats directly
       Stats is matrix of nLabels rows and 4 columns containing the following statistics:
            stats[n][0]: Number of pixels corresponding to label n (i.e. area)
            stats[n][1]: Weighted sum of label n in x direction ( sum(xi*pi) )
            stats[n][2]: Weighted sum of label n in y direction ( sum(yi*pi) )
            stats[n][3]: Sum of pixel values of label n ( sum(pi) )
            n: [0, nLabels-1]

            Hence the weighted centroid for label n is:
            x = stats[n][1] / stats[n][3];
            y = stats[n][2] / stats[n][3];
     */
    unsigned nLabels = cv::connectedComponentsForStarCam(mThreshed, mLabels,stats, 8, CV_16U);


    for(unsigned i=1; i<nLabels; ++i)
    {
        int * row = (int*) stats.ptr(i);
        if(*row > 16)
        {
            float x = 1.0 * row[1] / row[3];
            float y = 1.0 * row[2] / row[3];
            mSpots.push_back(Spot(cv::Point2f(x,mFrame.rows - y), *row) );
        }

    }
    return mSpots.size();
}

Eigen::Vector2f StarCamera::undistortRadialTangential(Eigen::Vector2f in) const
{
    float k1 = mDistortionCoeffi(0);
    float k2 = mDistortionCoeffi(1);
    float k3 = mDistortionCoeffi(4);
    float p1 = mDistortionCoeffi(2);
    float p2 = mDistortionCoeffi(3);

    float r2;
    float r4;
    float kRadial;
    Eigen::Vector2f Xc = in; // initial guess
    for(int i=0; i<20; ++i)
    {
        r2 = Xc.squaredNorm();
        r4 = r2*r2;
        kRadial = 1 + k1 * r2 + k2 * r4 + k3 * r2*r4;
        Eigen::Vector2f deltaX;
        deltaX << 2 * p1 * Xc(0) * Xc(1) + p2 *(r2 + 2 * Xc(0)*Xc(0)),
                  p1 * (r2 + 2 * Xc(1)*Xc(1)) + 2 * p2 * Xc(0) * Xc(1);

        Xc = (in - deltaX) / kRadial;
    }
    return Xc;
}

void StarCamera::computeWeightedCentroid(Contour_t &contour, cv::Point2f &centroid)
{
    /*
     * Steps:
     *  1. Get bounding rectangle frome contour
     *  2. Create temporary matrix with size of bounding rectangle
     *  3. draw contour into small matrix and fill it
     *  4. make AND operation with rectangle in original frame to copy contour area into empty matrix
     *  5. calculate weighted centroid by summing
     */

    // Get bounding rectangle from contour
    cv::Rect rect = cv::boundingRect(contour);


    // Create a temporary matrix with size of bounding rectangle
    cv::Mat temp(rect.size(), CV_8U, cv::Scalar(0));

    cv::Mat temp2 = mFrame(rect);

    std::vector<Contour_t> T = std::vector<Contour_t>(1,contour);
    // draw contour into temporary matrix
    cv::drawContours(temp, T, -1, cv::Scalar(255), cv::FILLED, 8, cv::noArray(), INT_MAX, cv::Point(-rect.tl()) );

    // make AND operation with rectangle and original image
    cv::bitwise_and(temp,temp2, temp);


    int sum = 0, weightingX = 0, weightingY = 0;
    for (int i=0; i<temp.rows; ++i)
    {
        u_int8_t *data = temp.ptr(i);

        for(int j=0; j<temp.cols; ++j)
        {
            sum += data[0];
            weightingX += j*data[0];
            weightingY += i*data[0];
        }
    }

    float weightedX = 1.0 * weightingX / sum;
    float weightedY = 1.0 * weightingX / sum;

    centroid.x = weightedX + rect.tl().x;
    centroid.y = mFrame.rows - (weightedY + rect.tl().y);
}

void StarCamera::computeWeightedCentroidBoundingRect(StarCamera::Contour_t &contour, cv::Point2f &centroid, unsigned &area)
{
    /*
     * Steps:
     *  1. Get bounding rectangle frome contour
     *  2. calculate weighted centroid by summing
     */

    // Get size of bounding rectangle from contour
    cv::Rect rect = cv::boundingRect(contour);

    // Create object to access values within bounding rectangle
    cv::Mat roi = mFrame(rect);

    // Calculate weighted sum
    int sum = 0, weightingX = 0, weightingY = 0;
    for (int i=0; i<roi.rows; ++i)
    {
        u_int8_t *data = roi.ptr(i);

        for(int j=0; j<roi.cols; ++j)
        {
            sum += data[0];
            weightingX += j*data[0];
            weightingY += i*data[0];
        }
    }

    float weightedX = 1.0 * weightingX / sum;
    float weightedY = 1.0 * weightingY / sum;


    centroid.x = weightedX + rect.tl().x;
    centroid.y = mFrame.rows - (weightedY + rect.tl().y);
    area = rect.width * rect.height;
}

