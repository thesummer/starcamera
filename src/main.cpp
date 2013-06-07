#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

#include "starcamera.h"

using namespace std;

int main(int argc, char **argv)
{

    if(argc < 2)
    {
        cout << "Error: no filename for raw file" << endl;
        return 1;
    }

    StarCamera starCam;

    starCam.getImageFromFile(argv[1]);

    starCam.extractSpots();

    starCam.extractSpots();

    vector<StarCamera::Spot>::iterator it;
    for(it = starCam.mSpots.begin(); it != starCam.mSpots.end(); ++it)
    {
        cv::circle(starCam.mFrame, it->center, it->radius, cv::Scalar(255), 2);
    }

    cv::imwrite("test.png", starCam.mFrame);

    return 0;
}

