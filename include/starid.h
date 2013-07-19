#ifndef STARID_H
#define STARID_H

#include<sqlite3.h>
#include<string>
#include<vector>
#include<utility>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "datatypes.h"


class StarIdentifier
{
public:
    struct Feature2
    {
        Feature2() {}
        Feature2(int id1_, int id2_, float theta_)
            :id1(id1_), id2(id2_), theta(theta_) {}
        int id1;
        int id2;
        float theta;
    };

    typedef std::pair<int,int> Star; // first = id in spotlist, second = hip

    typedef std::vector<StarIdentifier::Feature2> featureList_t;
    typedef std::vector<Eigen::Vector3f> vectorList_t;

    StarIdentifier();

    ~StarIdentifier();

    void setFeatureListDB(const char* filename);

    void openDb();

    void loadFeatureListKVector(const char* filename);

    void identify2StarMethod(const vectorList_t& starVectors, float eps=1e-2);

    void identifyPyramidMethod(const vectorList_t& starVectors, float eps=1e-2);

private:

    void createFeatureList2(const vectorList_t &starVectors,
                            featureList_t &output) const;

    void retrieveFeatureList(sqlite3_stmt *sqlStmt, StarIdentifier::featureList_t &output) const;
    std::string mDbFile;
    sqlite3 * mDb;
    bool mOpenDb;
    std::vector<Feature2>  mFeatureList;
    std::vector<int> mKVector;
    double mQ;
    double mM;
};

#endif // STARCAMERA_H
