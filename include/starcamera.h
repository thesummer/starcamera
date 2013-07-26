#ifndef STARCAMERA_H
#define STARCAMERA_H

#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "datatypes.h"
#include "aptina.h"

class StarCamera
{
public:

    typedef std::vector<cv::Point> Contour_t;

    StarCamera();

    void initializeCamera();

    void getImage();

    void getImageFromFile(const char *filename, int rows=1944, int cols=2592);

    int extractSpots();

    int WeightedCentroiding();

    int WeightedCentroidingBoundingRect();

    int ConnectedComponents();

    int ConnectedComponentsWeighted();

    void calculateSpotVectors();

    void loadCalibration(const char *filename);

    int getThreshold() const {return mThreshold; }
    void setThreshold(int value) { mThreshold = value; }

    float getMinRadius() const { return mMinRadius; }
    void setMinRadius(float value) { mMinRadius = value;}

    const std::vector<Spot>& getSpots() const {return mSpots;}

    const std::vector<Eigen::Vector3f>& getSpotVectors() const {return mSpotVectors;}

    void cameraTest();

    int mThreshold;
    float mMinRadius;
    cv::Mat_<u_int8_t> mFrame;
    cv::Mat_<u_int8_t> mThreshed;
    cv::Mat_<u_int16_t> mLabels;
    unsigned int mMinArea;




private:
    std::vector<Spot> mSpots;
    std::vector<Eigen::Vector3f> mSpotVectors;
    Eigen::Matrix3f mCamMatrix;
    Eigen::Matrix<float,5,1> mDistortionCoeffi;
    Eigen::Vector2f mPrincipalPoint;
    Eigen::Vector2f mFocalLength;
    float mPixelSkew;
    Aptina mCamera;

    Eigen::Vector2f undistortRadialTangential(Eigen::Vector2f in) const;
    void computeWeightedCentroid(Contour_t &contour, cv::Point2f &centroid);
    void computeWeightedCentroidBoundingRect(Contour_t &contour, cv::Point2f &centroid, int &area);
};

#endif // STARCAMERA_H
