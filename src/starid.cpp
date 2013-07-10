#include "starid.h"

StarIdentifier::StarIdentifier()
{
}

StarIdentifier::~StarIdentifier()
{
    // close database (if open) before destroying the object
    sqlite3_close(mDb);
}

void StarIdentifier::setFeatureListDB(const char *filename)
{
    mDbFile = filename;
}

bool StarIdentifier::openDb()
{
    if (mDbFile.empty())
        return false; // throw Error?

    // close old database (if any was open)
    sqlite3_close(mDb);

    // open new database
    if (sqlite3_open(mDbFile.c_str(), &mDb) != SQLITE_OK)
    {
        return false; // throw Error?
    }

    return true;
}

void StarIdentifier::identifyStars(const std::vector<Eigen::Vector3f>& starVectors)
{

}

void StarIdentifier::createFeatureList2(const std::vector<Eigen::Vector3f> &starVectors, std::vector<StarIdentifier::Feature2>& output) const
{
    output.clear();

    std::vector<Eigen::Vector3f>::const_iterator it1, end1, it2;
    end1 = starVectors.end() -1 ;
    int i = 0;
    for(it1 = starVectors.begin(); it1 != end1; ++it2, ++i)
    {
        int j = 0;
        for( it2 = it1+1; it2!= starVectors.end(); ++it2, ++j)
        {
            float theta = acos(it1->dot(*it2) / (it1->norm() * it2->norm()) );
            output.push_back(Feature2(i, j, theta) ) ;
        }
    }
}



