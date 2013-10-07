#ifndef STARCAMERA_H
#define STARCAMERA_H

#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <vector>
#include <string>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "datatypes.h"
#include "aptina.h"

/*!
 \brief

*/
class StarCamera
{
public:

    /*!
     \brief Typedef for which represents a contour in OpenCV
    */
    typedef std::vector<cv::Point> Contour_t;

    /*!
     \brief Enumeration of the different methods for centroiding

     More detailed information about each method can be found in
     CentroidingContours(), CentroidingConnectedComponentsGeometric()
     and CentroidingConnectedComponentsWeighted()
    */
    enum CentroidingMethod
    {
        ContoursGeometric,
        ContoursWeighted,
        ContoursWeightedBoundingBox,
        ConnectedComponentsGeometric,
        ConnectedComponentsWeighted
    };

    /*!
    \brief Constructor

    Creates an empty StarCamera instance
    */
    StarCamera();

    /*!
     \brief Initializes a connected Aptina camera

     \param initFile Initialization file for the Aptina
    */
    void initializeCamera(const std::string initFile = NULL);

    /*!
     \brief Grab a frame from the Aptina camera
    */
    void getImage();

    /*!
     \brief Load a raw image from file
     The image has to be in Bayer-12 format with 12-bit resolution
     being stored in 2 bytes with leading 0s

     Defaults for height and width of the image correspond to the
     5 MP Aptina camera.

     \param filename Filename
     \param rows Width of the image
     \param cols Height of the image
    */
    void getImageFromFile(const std::string filename, const unsigned rows=1944, const unsigned cols=2592);

    /*!
     \brief Extracts the star spots from the previously loaded image

    Use setThreshold() and setMinArea() to set main parameters.

     \param method Method to be used for determining spots from noise and centroiding
     \return unsigned Number of extracted spots
    */
    unsigned extractSpots(CentroidingMethod method = ConnectedComponentsWeighted);

    /*!
     \brief Calulates the Vectors in Camera frame for each spot extracted in an image

    */
    void calculateSpotVectors();

    /*!
     \brief Loads the calibration file for the Aptina camera

     \param filename
    */
    void loadCalibration(const std::string filename);

    /*!
     \brief Return the current threshold setting for spot extraction

     \return unsigned
    */
    unsigned getThreshold() const {return mThreshold; }

    /*!
     \brief Set the threshold for spot extraction

     \param value
    */
    void setThreshold(unsigned value) { mThreshold = value; }


    /*!
     \brief Return the current minimum Area a spot has to cover to count as star
     \return unsigned
    */
    unsigned getMinArea() const {return mMinArea; }

    /*!
     \brief Set the minimum Area a spot has to cover to count as star

     \param value
    */
    void setMinArea(unsigned value) {mMinArea = value; }


    /*!
     \brief Returns a const reference to the vector of extracted Spots

     TODO:
        Maybe make another option to get a copy instead of a reference?

     \return const std::vector<Spot>
    */
    const std::vector<Spot>& getSpots() const {return mSpots;}

    /*!
     \brief Returns a const reference to the vector of camera vectors of extrated Spots

     TODO:
        Maybe make another option to get a copy instead of a reference?

     \return const std::vector<Eigen::Vector3f>
    */
    const std::vector<Eigen::Vector3f>& getSpotVectors() const {return mSpotVectors;}

    /*!
     \brief Test routine to check if the camera is working

     Tries to connect to the Aptina camera and make some example pictures
    */
    void cameraTest();

    cv::Mat_<u_int8_t> mFrame; /*!< Object which contains the current frame*/
    cv::Mat_<u_int8_t> mThreshed; /*!< Object which contains the current frame after applying a Threshold*/
    cv::Mat_<u_int8_t> mTemp;



private:
    cv::Mat_<u_int16_t> mLabels; /*!< TODO: Useful? */
    unsigned mThreshold; /*!< Threshold under which pixels will be seen as black*/
    unsigned int mMinArea; /*!< Lowest area under which a star spot will be treated as noise*/
    std::vector<Spot> mSpots; /*!< Vector containing all identified spots from the last run of extractSpots()*/
    std::vector<Eigen::Vector3f> mSpotVectors; /*!< Vector containing all vectors computed in the last run of calculateSpotVectors()*/
    Eigen::Matrix<float,5,1> mDistortionCoeffi; /*!< Matrix containing the coefficients to compensate lense distortion*/
    Eigen::Vector2f mPrincipalPoint; /*!< 2D-coordinates of the lense's principal point determined from calibration procedure*/
    Eigen::Vector2f mFocalLength; /*!< 2D focal length of the lense determined in calibration procedure*/
    float mPixelSkew; /*!< Pixel skew of the camera determined in calibration procedure*/
    Aptina mCamera; /*!< Representation of the Aptina camera to grab images from*/


    /*!
     \brief Extract spots from image using OpenCVs findContours method and a centroiding method

     Possible centroiding methods are ContoursGeometric, ContoursWeighted, ContoursWeightedBoundingBox
     ContoursGeometric:
        From the determined contours get the minimum enclosing circle. Center of a Spot is the center of
        the circle and area is the area of this circle.

     ContoursWeighted:
        Fill the determined contours and calculate the weighted centroid.
        Center of a spot is the computed centroid and area is the number of pixels in the filled contour.

     ContoursWeightedBoundingBox:
        Get the bounding rectangle of the contour and calculate the weighted centroid of the all pixels
        within it. The center of a spot is the computed centroid and the area is the area of the rectangle.


     \param method Method which is used.
     \return unsigned
    */
    unsigned CentroidingContours(CentroidingMethod method);

    /*!
     \brief Extracts spots from image using OpendCVs (new) connectedComponents method

     Uses the statistics the connectedComponents function produces to determine the
     center and the area of the Spot.

     \return unsigned
    */
    unsigned CentroidingConnectedComponentsGeometric();

    /*!
     \brief Extracts spots from image using an altered version OpendCVs (new) connectedComponents method

     Uses the statistics produced by this function to calculate the weighted centroid
     and the area of each Spot

     \return unsigned
    */
    unsigned CentroidingConnectedComponentsWeighted();

    /*!
     \brief Computes the weighted centroid for a given contour

     \param contour Reference to the contour
     \param centroid
    */
    int computeWeightedCentroid(Contour_t &contour, cv::Point2f &centroid);
    /*!
     \brief Computes the weighted centroid and area for a given contour using the bounding rectangle

     \param contour Reference to the contour
     \param centroid
     \param area
    */
    void computeWeightedCentroidBoundingRect(Contour_t &contour, cv::Point2f &centroid, unsigned &area);

    /*!
     \brief Compensates centroids of Spots for the lens distortion effects

     \param in Original Spot centroid
     \return Eigen::Vector2f Spot centroid after compensation for lens distortion
    */
    Eigen::Vector2f undistortRadialTangential(const Eigen::Vector2f in) const;
};

#endif // STARCAMERA_H
