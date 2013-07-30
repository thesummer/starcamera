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

/*!
 \brief A representation of the feature between two stars in the catalog

 In this case the feature is the angular displacement theta
 between 2 stars with id1 and id2.
*/
struct Feature2
{
    /*!
    \brief Constructs an empty Feature2 instance
    */
    Feature2() {}

    /*!
    \brief Constructs a Feature2 from predefined values

    \param id1_
    \param id2_
    \param theta_
    */
    Feature2(int id1_, int id2_, float theta_)
        :id1(id1_), id2(id2_), theta(theta_) {}
    int id1; /*!< (hip-) id of the first star */
    int id2; /*!< (hip-) id of the second star*/
    float theta; /*!< Anglular displacement between the two stars*/
};

#endif
