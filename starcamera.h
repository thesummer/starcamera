#ifndef STARCAMERA_H
#define STARCAMERA_H

#include <opencv2/core/core.hpp>
#include <stdint.h>

class StarCamera
{
public:
    StarCamera();

    void getImage();

    void getImageFromFile(const char *filename, int rows, int cols);

    int extractSpots();

    void calculateSpotVectors();

    cv::Mat_<u_int8_t> mFrame;
};

#endif // STARCAMERA_H
