#include <fstream>
#include <opencv2/imgproc/imgproc.hpp>

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
    std::vector< std::vector<cv::Point> > contours;
    // Find contours in the threshed image
    cv::findContours(mThreshed, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

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
            mSpots.push_back(Spot(*it, center, radius, area));
        }
    }

    return mSpots.size();
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

