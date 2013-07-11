#include<map>

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

void StarIdentifier::identifyStars(const vectorList_t &starVectors, float eps)
{
    // create a feature list from the star vectors
    typedef std::vector<StarIdentifier::Feature2> featureList_t;
    featureList_t featureList;
    createFeatureList2(starVectors, featureList);

    // Vector which holds a map with possible hip for each spot
    typedef std::vector<std::map<int, int> > idTable_t ;
    idTable_t idTable(starVectors.size(), std::map<int,int> );

    // prepare a statement for the database which searches for the feature/angle within an interval
    std::string sql("SELECT * FROM featureList WHERE theta >? AND theta < ?");
    sqlite3_stmt * sqlStmt;
    if (sqlite3_prepare_v2(mDb, sql.c_str(), sql.size()+1, &sqlStmt, 0) != SQLITE_OK)
        ;// throw error

    /* Algorithm:
     *  1. Take feature from featureList (done using iterator)
     *  2. search in db for the feature within an interval of 2 epsilon (+-)
     *  3. add all possible hip to the star spot(s) corresponding to the feature (or increase the counter if already existant) using the map
     *  4. Goto 1.
     *  5. For each spots take the hip with the highest counter
     */
    for(featureList_t::const_iterator it = featureList.begin(); it != featureList.end(); ++it)
    {
       float low  = it->theta - eps; // lower bound of interval
       float high = it->theta + eps; // upper bound of interval

       // bind upper and lower bound to the sql statement
       sqlite3_bind_double(sqlStmt, 1, low);
       sqlite3_bind_double(sqlStmt, 2, high);


       /// TODO: maybe use sqlite_execute instead with a callback function?
       // execute the query and iterate through the result
       int result;
       while( (result = sqlite3_step(sqlStmt)) == SQLITE_ROW)
       {
           // get the first hip
           int index = sqlite3_column_int(sqlStmt, 0);

           // increase the counter for the hip on both star spots as the problem is symmetric
           // NOTE: an explicit insert for a new hip id is not necessary
           // call is idTable[index of the first spot][key for hip]
           idTable[it->id1][index]++;
           idTable[it->id2][index]++;

           // get second hip and increase counter
           index = sqlite3_column_int(sqlStmt, 1);
           idTable[it->id1][index]++;
           idTable[it->id2][index]++;
       }

       if (result != SQLITE_DONE)
           ; // throw error
    }

    // 5. determine the hip for each star spot
    std::vector<Star> starList;
    int i=0;
    for(idTable_t::const_iterator it = idTable.begin(); it != idTable.end(); ++it, ++i)
    {
        int max = -1;
        int hip = 0;
        bool unique = true;
        for(std::map<int,int>::const_iterator itMap = it->begin(); itMap != it->end(); ++itMap)
        {
            if(max == itMap->second)
            {
                unique = false;
            }
            else
            {
                max = std::max(max, itMap->second);
                hip = itMap->first;
                unique = true;
            }
        }
        if (unique)
        {
            starList.push_back(Star(i, hip));
        }
        else
        {
            starList.push_back(Star(i, -1));
        }

    }
}

void StarIdentifier::createFeatureList2(const vectorList_t &starVectors, featureList_t &output) const
{
    // start from the beginning
    output.clear();


    vectorList_t::const_iterator it1, end1, it2;
    end1 = starVectors.end() -1 ;

    /* NOTE: i and j are only for counting purposes.
     *       they also could be used for direct accessing the vector elements
     *       but iterators clutter the code less  */
    int i = 0;
    for(it1 = starVectors.begin(); it1 != end1; ++it2, ++i)
    {
        int j = i+1;
        for( it2 = it1+1; it2!= starVectors.end(); ++it2, ++j)
        {
            // compute the angle between the two vectors and add to the feature list
            float theta = acos(it1->dot(*it2) / (it1->norm() * it2->norm()) );
            output.push_back(Feature2(i, j, theta) ) ;
        }
    }
}



