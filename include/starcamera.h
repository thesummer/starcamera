#ifndef STARCAMERA_H
#define STARCAMERA_H

#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

class StarCamera
{
public:

    struct Spot
    {
        Spot();
        Spot(std::vector<cv::Point> contour_, cv::Point2f center_, float radius_)
            :contour(contour_), center(center_), radius(radius_) {}
        std::vector<cv::Point> contour;
        cv::Point2f center;
        float radius;
    };

    StarCamera();

    void getImage();

    void getImageFromFile(const char *filename, int rows=1944, int cols=2592);

    int extractSpots();

    void calculateSpotVectors();

    void loadCalibration(const char *filename);

    int getThreshold() const {return mThreshold; }
    void setThreshold(int value) { mThreshold = value; }

    float getMinRadius() const { return mMinRadius; }
    void setMinRadius(float value) { mMinRadius = value;}

    int mThreshold;
    float mMinRadius;
    cv::Mat_<u_int8_t> mFrame;
    cv::Mat_<u_int8_t> mThreshed;
    std::vector<Spot>  mSpots;


private:
    std::vector<Eigen::Vector3f> mSpotVectors;
    Eigen::Matrix3f mCamMatrix;
    Eigen::Matrix<float,5,1> mDistortionCoeffi;
    Eigen::Vector2f mPrincipalPoint;
    Eigen::Vector2f mFocalLength;
    float mPixelSkew;

    Eigen::Vector2f undistortRadialTangential(Eigen::Vector2f in) const;
};

#endif // STARCAMERA_H
