#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>

#include "starcamera.h"

StarCamera::StarCamera()
    :mThreshold(64)
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
    file.open(filename, ios_base::in | ios_base::binary);
    if(!file.is_open())
    {
        /// TODO throw error
    }

    // resize matrix structure for image data if necessary
    mFrame.create(rows, cols);

    // read image data and transform from 12 to 8 bit
    /// TODO: Check if reading from file could be made more efficient
    for (int i = 0; i<rows; ++i)
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

}

int StarCamera::extractSpots()
{
    /// TODO: extract spots from the image using OpenCV
    mSpots.clear();


    if(mFrame.data)
    {
        /// TODO error when no frame is loaded
    }

    // Threshold the image: set all pixels lower than mThreshold to 0
    cv::threshold(mFrame, mThreshed, mThreshold, 0, cv::THRESH_TOZERO);

    // vector which stores the point belonging to each contour
    std::vector< std::vector<cv::Point> > contours;
    // Find contours in the threshed image
    cv::findContours(mThreshed, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    // Find matching contours/spots
    std::vector <std::vector<cv::Point> >::iterator it;
    for (it = contours.begin(); it != contours.end(); ++it)
    {
        Point2f center;
        float radius;

        // find the circle for each contour
        cv::minEnclosingCircle(Mat(*it), center, radius);

        // Save the spot if it is large enough
        if(radius > mMinRadius)
        {
            Spot temp;
            temp.center = center;
            temp.contour = *it;
            temp.radius = radius;

            mSpots.push_back(temp);
        }
    }
}

void StarCamera::calculateSpotVectors()
{
    /// TODO: apply lens correction
    /// TODO: calculate the vectors from the image
}

