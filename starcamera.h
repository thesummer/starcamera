#ifndef STARCAMERA_H
#define STARCAMERA_H

#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <vector>

class StarCamera
{
public:

    struct Spot
    {
        std::vector<cv::Point> contour;
        cv::Point2f center;
        float radius;
    };

    StarCamera();

    void getImage();

    void getImageFromFile(const char *filename, int rows, int cols);

    int extractSpots();

    void calculateSpotVectors();

    int getThreshold() const {return mThreshold; }
    void setThreshold(int value) { mThreshold = value; }

    float getMinRadius() const { return mMinRadius; }
    void setMinRadius(float value) { mMinRadius = value;}

    int mThreshold;
    float mMinRadius;
    cv::Mat_<u_int8_t> mFrame;
    cv::Mat_<u_int8_t> mThreshed;
    std::vector<Spot>  mSpots;


};

#endif // STARCAMERA_H
