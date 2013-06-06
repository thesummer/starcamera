#ifndef STARCAMERA_H
#define STARCAMERA_H

class StarCamera
{
public:
    StarCamera();

    void getImage();

    void getImageFromFile(const char *filename);

    int extractSpots();

    void calculateSpotVectors();
};

#endif // STARCAMERA_H
