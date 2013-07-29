#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <opencv2/core/core.hpp>

/*!
 \brief Representation of a star spot

 A spot represents the 2-dimensional coordinates of a bright
 spot in a star image and its area.

*/
struct Spot
{
/*!
 \brief Constructs an empty Spot
*/
    Spot() {}

/*!
 \brief Constructs a Spot from predefined values

 \param center_ Centroid of the Spot (2D)
 \param area_   Area (in px) of the Spot
*/
    Spot(cv::Point2f center_, float area_)
        :center(center_), area(area_) {}
    cv::Point2f center; /*!< Centroid of the Spot (2D) */
    unsigned area; /*!< Area (in px) of the Spot */
};

#endif
