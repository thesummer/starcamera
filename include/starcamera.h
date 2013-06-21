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

    typedef std::vector<cv::Point> Contour_t;
    struct Spot
    {
        Spot();
        Spot(Contour_t contour_, cv::Point2f center_, float radius_, int area_, cv::Point2f centroid1_, cv::Point2f centroid2_)
            :contour(contour_), center(center_), radius(radius_), area(area_), centroid1(centroid1_), centroid2(centroid2_) {}
        std::vector<cv::Point> contour;
        cv::Point2f center;
        cv::Point2f centroid1;
        cv::Point2f centroid2;
        float radius;
        int area;

    };

    struct Spot2
    {
        Spot2() {}
        Spot2(cv::Point2d center_, float area_)
            :center(center_), area(area_) {}
        cv::Point2d center;
        int area;
    };

    StarCamera();

    void getImage();

    void getImageFromFile(const char *filename, int rows=1944, int cols=2592);

    int extractSpots();

    int testConnectedComponents();

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
    cv::Mat_<u_int8_t> mBw;
    cv::Mat_<u_int16_t> mLabels;
    std::vector<Spot>  mSpots;

    std::vector<Spot2> mTestSpots;

    unsigned int mMinArea;


private:
    std::vector<Eigen::Vector3f> mSpotVectors;
    Eigen::Matrix3f mCamMatrix;
    Eigen::Matrix<float,5,1> mDistortionCoeffi;
    Eigen::Vector2f mPrincipalPoint;
    Eigen::Vector2f mFocalLength;
    float mPixelSkew;

    Eigen::Vector2f undistortRadialTangential(Eigen::Vector2f in) const;
    void computeWeightedCentroid(Contour_t &contour, cv::Point2f &centroid1, cv::Point2f &centroid2);
};

#endif // STARCAMERA_H
