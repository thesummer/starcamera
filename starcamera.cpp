#include <fstream>

#include "starcamera.h"

StarCamera::StarCamera()
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
}

int StarCamera::extractSpots()
{
    /// TODO: extract spots from the image using OpenCV
}

void StarCamera::calculateSpotVectors()
{
    /// TODO: apply lens correction
    /// TODO: calculate the vectors from the image
}



