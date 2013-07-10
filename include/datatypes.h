#ifndef DATA_TYPES_H
#define DATA_TYPES_H

struct Spot
{
    Spot() {}
    Spot(cv::Point2f center_, float area_)
        :center(center_), area(area_) {}
    cv::Point2f center;
    int area;
};

#endif
