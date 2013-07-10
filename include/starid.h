#ifndef STARID_H
#define STARID_H

#include<sqlite3.h>
#include<string>
#include<vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "datatypes.h"


class StarIdentifier
{
    struct Feature2
    {
        Feature2() {}
        Feature2(int hip1_, int hip2_, float theta_)
            :hip1(hip1_), hip2(hip2_), theta(theta_) {}
        int hip1;
        int hip2;
        float theta;
    };

    StarIdentifier();

    ~StarIdentifier();

    void setFeatureListDB(const char* filename);

    bool openDb();

    void identifyStars(const std::vector<Eigen::Vector3f>& starVectors);

private:

    void createFeatureList2(const std::vector<Eigen::Vector3f> &starVectors,
                            std::vector<Feature2> &output) const;

    std::string mDbFile;
    sqlite3 * mDb;
};

#endif // STARCAMERA_H
