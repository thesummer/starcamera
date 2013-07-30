#ifndef STARID_H
#define STARID_H

#include<sqlite3.h>
#include<string>
#include<vector>
#include<utility>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "datatypes.h"


/*!
 \brief

*/
class StarIdentifier
{
public:
    enum IdentificationMethod
    {
        TwoStar,
        PyramidSQL,
        PyramidKVector
    };

    /*!
     \brief Typedef to store the link between the index in a list and the hip of a star

     Used?
    */
    typedef std::pair<int,int> Star; // first = id in spotlist, second = hip

    /*!
     \brief Typedef for a featureList
    */
    typedef std::vector<Feature2> featureList_t;

    /*!
     \brief Typedef for a list of 3D-Vectors
    */
    typedef std::vector<Eigen::Vector3f> vectorList_t;

    /*!
    \brief Constructor

    Creates an empty StarIdentifier instance
    */
    StarIdentifier();

    /*!
     \brief Destructor
    */
    ~StarIdentifier();

    /*!
     \brief Sets the database file containing the featurelist

     \param filename SQLite-database file
    */
    void setFeatureListDB(const char* filename);

    /*!
     \brief Opens the database file and initializes the SQLite handle

     TODO:
        load the file into memory to get faster IO than reading from disk?

    */
    void openDb();

    /*!
     \brief Loads the FeatureList as from file and saves it as k-Vector

     Note:
        k-Vector technique is described by Mortari

     \param filename
    */
    void loadFeatureListKVector(const char* filename);

    /*!
     \brief Identify the star using the specified identification method

     \param starVectors Input Vectors extracted from an image
     \param eps Allowed tolerance when comparing features (in degree)
     \param method The method to use for identification
     \return std::vector<int> Vector of hip-IDs which correspond to the elements of starVectors
    */
    std::vector<int> identifyStars(const vectorList_t &starVectors, const float eps, IdentificationMethod method = PyramidKVector) const;

private:

    /*!
     \brief Star identification using the 2-star method

     Uses the SQLite database for searching through the feature list

     \param starVectors vector of star vectors
     \param eps the tolerance for feature matching in degrees
    */
    std::vector<int> identify2StarMethod(const vectorList_t& starVectors, const float eps=1e-2) const;

    /*!
     \brief Star identification using Pyramid method

     Uses the SQLite database for searching through the feature list

     \param starVectors vector of star vectors
     \param eps the tolerance for feature matching in degrees
     \return std::vector<int>
    */
    std::vector<int> identifyPyramidMethod(const vectorList_t& starVectors, const float eps=1e-2) const;

    /*!
     \brief Star identification using Pyramid method

     Uses the k-Vector technique to search through the feature list

     \param starVectors vector of star vectors
     \param eps the tolerance for feature matching in degrees
     \return std::vector<int>
    */
    std::vector<int> identifyPyramidMethodKVector(const vectorList_t& starVectors, const float eps=1e-2) const;

    /*!
     \brief Create a list of all unique features between the stars in starVectors

     \param starVectors Input vectors of stars
     \param output List of features between each 2 Vectors (without mirror combinations)
    */
    void createFeatureList2(const vectorList_t &starVectors,
                            featureList_t &output) const;

    /*!
     \brief Run an SQL-statement and read the results into a featureList_t object

     \param sqlStmt SQL-statement to evaluate
     \param output FeatureList containing all possible pairs which match
    */
    void retrieveFeatureList(sqlite3_stmt *sqlStmt, StarIdentifier::featureList_t &output) const;

    /*!
     \brief Get all features which are within thetaMin and thetaMax

     \param thetaMin Minimum angle between two stars
     \param thetaMax Maximum angle between two stars
     \param output FeatureList containing all possible pairs that match
    */
    void retrieveFeatureListKVector(float thetaMin, float thetaMax, StarIdentifier::featureList_t &output) const;

    /*!
     \brief Get all features which are within thetaMin and thetaMax and contain hip

     \param thetaMin Minimum angle between two stars
     \param thetaMax Maximum angle between two stars
     \param hip hip-ID which has to match one star in pair
     \param output FeatureList containing all possible pairs that match
    */
    void retrieveFeatureListKVector(float thetaMin, float thetaMax, int hip, StarIdentifier::featureList_t &output) const;

    std::string mDbFile; /*!< Filename of the database file */
    sqlite3 * mDb; /*!< SQLite database handle*/
    bool mOpenDb; /*!< Database is opened or closed*/
    std::vector<Feature2>  mFeatureList; /*!< Sorted feature list for k-Vector technique*/
    std::vector<int> mKVector; /*!< k-Vector for k-Vector technique*/
    double mQ; /*!< Parameter q for k-Vector technique*/
    double mM; /*!< Parameter m for k-Vector technique*/
};

#endif // STARCAMERA_H
