#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/utility.hpp>

#include <iostream>
using std::cout;
using std::endl;

#include "starcamera.h"

StarCamera::StarCamera()
    :mThreshold(64), mMinRadius(4.0f)
{

}

void StarCamera::getImage()
{
    /// TODO: grap a frame from Aptina using Apbase library
}

void StarCamera::getImageFromFile(const char* filename, int rows, int cols)
{
    // open image file
    std::fstream file;
    file.open(filename, std::ios_base::in | std::ios_base::binary);
    if(!file.is_open())
    {
        /// TODO throw error
    }

    // resize matrix structure for image data if necessary
    mFrame.create(rows, cols);

    // read image data and transform from 12 to 8 bit
    /// TODO: Check if reading from file could be made more efficient
    for (int i = rows-1; i>0; --i)
    {
        for(int j = 0; j<cols; ++j)
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

int StarCamera::extractSpots()
{
    mSpots.clear();


    if(mFrame.data)
    {
        /// TODO error when no frame is loaded
    }

    // Threshold the image: set all pixels lower than mThreshold to 0
    cv::threshold(mFrame, mThreshed, mThreshold, 0, cv::THRESH_TOZERO);

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

        // find the circle for each contour
        cv::minEnclosingCircle(*it, center, radius);

        // get the area of each contour
        unsigned int area = (int) cv::contourArea(*it);

        // Save the spot if it is large enough
        if(radius > mMinRadius)
//        if (area > mMinArea)
        {
            cv::Point2f centroid1, centroid2;
            computeWeightedCentroid(*it, centroid1, centroid2);
            mSpots.push_back(Spot(*it, center, radius, area, centroid1, centroid2));
        }
    }


    return mSpots.size();
}

int StarCamera::testConnectedComponents()
{
    mTestSpots.clear();

    mBw = mFrame > mThreshold;
    cv::Mat stats;
    cv::Mat centroids;
    int nLabels = cv::connectedComponentsWithStats(mBw, mLabels, stats, centroids, 8, CV_16U);

    for(int i=1; i<nLabels; ++i)
    {
        int indexArea = 4;
        int area = stats.at<int>(i, indexArea);
        if(area > 18)
        {
            mTestSpots.push_back(Spot2(centroids.at<cv::Point2d>(i),area));
        }
    }

    return nLabels;
}

void StarCamera::calculateSpotVectors()
{
    /// TODO: apply lens correction
    /// TODO: calculate the vectors from the image

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
        if(mDistortionCoeffi.norm() != 0.0f)  // Use epsilon environment?
        {
            Xd = undistortRadialTangential(Xd);

        }
        spotVec << Xd(0), Xd(1), 1.0f;
        spotVec.normalize();
        mSpotVectors.push_back(spotVec);
    }
}

void StarCamera::loadCalibration(const char *filename)
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

Eigen::Vector2f StarCamera::undistortRadialTangential(Eigen::Vector2f in) const
{
    float k1 = mDistortionCoeffi(0);
    float k2 = mDistortionCoeffi(1);
    float k3 = mDistortionCoeffi(2);
    float p1 = mDistortionCoeffi(3);
    float p2 = mDistortionCoeffi(4);

    for(int i=0; i<20; ++i)
    {
        float r2 = in.squaredNorm();
        float r4 = r2*r2;
        float kRadial = 1 + k1 * r2 + k2 * r4 + k3 * r2*r4;
        Eigen::Vector2f deltaX;
        deltaX << 2 * p1 * in(0) * in(1) + p2 *(r2 + 2 * in(0)*in(0)),
                  p1 * (r2 + 2 * in(1)*in(1)) + 2 * p2 * in(0) * in(1);

        in = (in - deltaX) / kRadial;
    }
    return in;
}

void StarCamera::computeWeightedCentroid(Contour_t &contour, cv::Point2f &centroid1, cv::Point2f &centroid2)
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


    sum = 0, weightingX = 0, weightingY = 0;
    for (int i=0; i<temp2.rows; ++i)
    {
        u_int8_t *data = temp2.ptr(i);

        for(int j=0; j<temp2.cols; ++j)
        {
            sum += data[0];
            weightingX += j*data[0];
            weightingY += i*data[0];
        }
    }

    float weightedX2 = 1.0 * weightingX / sum;
    float weightedY2 = 1.0 * weightingY / sum;


    centroid1.x = weightedX + rect.tl().x;
    centroid1.y = mFrame.rows - (weightedY + rect.tl().y);

    centroid2.x = weightedX2 + rect.tl().x;
    centroid2.y = mFrame.rows - (weightedY2 + rect.tl().y);

//    cv::imwrite("test2.png", temp);
}

