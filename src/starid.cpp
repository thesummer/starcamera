#include<map>
#include<stdexcept>
#include<fstream>
#include<iostream>
using std::cout;
using std::endl;

#include "starid.h"


StarIdentifier::StarIdentifier()
    :mDb(NULL), mOpenDb(false)
{
}

StarIdentifier::~StarIdentifier()
{
    // close database (if open) before destroying the object
    sqlite3_close(mDb);
    mOpenDb = false;
}

void StarIdentifier::setFeatureListDB(const char *filename)
{
    mDbFile = filename;
}

void StarIdentifier::openDb()
{
    if (mDbFile.empty())
    {
        throw std::invalid_argument("No db-file speicfied");
    }
    // close old database (if any was open)
    sqlite3_close(mDb);

    // open new database
    if (sqlite3_open(mDbFile.c_str(), &mDb) != SQLITE_OK)
    {
        mOpenDb = false;
        throw std::runtime_error("Opening database failed");
    }

    mOpenDb = true;
}

void StarIdentifier::loadFeatureListKVector(const char *filename)
{
    mFeatureList.clear();
    mKVector.clear();

    std::ifstream ifile;
    ifile.open(filename);

    ifile >> mQ;
    ifile >> mM;

    do
    {
        int k, hip1, hip2;
        float theta;
        ifile >> k;
        ifile >> hip1;
        ifile >> hip2;
        ifile >> theta;

        mKVector.push_back(k);
        mFeatureList.push_back(Feature2(hip1, hip2, theta));
    } while(!ifile.eof());
}

void StarIdentifier::identify2StarMethod(const vectorList_t &starVectors, float eps)
{
    if(!mOpenDb)
        throw std::runtime_error("No Database opened");

    // create a feature list from the star vectors
    typedef std::vector<StarIdentifier::Feature2> featureList_t;
    featureList_t featureList;
    createFeatureList2(starVectors, featureList);

    // Vector which holds a map with possible hip for each spot
    typedef std::vector<std::map<int, int> > idTable_t ;
    idTable_t idTable(starVectors.size(), std::map<int,int>() );

    // prepare a statement for the database which searches for the feature/angle within an interval
    std::string sql("SELECT * FROM featureList WHERE theta >? AND theta < ?");
    sqlite3_stmt * sqlStmt;
    if (sqlite3_prepare_v2(mDb, sql.c_str(), sql.size()+1, &sqlStmt, 0) != SQLITE_OK)
        throw std::runtime_error("Prparing SQL search query failed");

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
       if(sqlite3_reset(sqlStmt) != SQLITE_OK)
           throw std::runtime_error("Resetting SQL query failed");

       if(sqlite3_bind_double(sqlStmt, 1, low) != SQLITE_OK)
           throw std::runtime_error("Binding new value1 to query failed");

       if(sqlite3_bind_double(sqlStmt, 2, high) != SQLITE_OK)
               throw std::runtime_error("Binding new value2 to query failed");


       /// TODO: maybe use sqlite_execute instead with a callback function?
       // execute the query and iterate through the result
       int result;
       while( (result = sqlite3_step(sqlStmt)) == SQLITE_ROW)
       {
           // get the first hip
           int index = sqlite3_column_int(sqlStmt, 0);

//           cout << "index: " << index << "  id1: " << it->id1 << "  id2: " << it->id2 << endl;

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
           throw std::runtime_error("SQL search returned with unexpected result");
    }

    int k = 0;
    for(idTable_t::const_iterator it = idTable.begin(); it != idTable.end(); ++k, ++it)
    {
        cout << k << ":" << endl;
        for(std::map<int,int>::const_iterator itMap = it->begin(); itMap != it->end(); ++itMap)
        {
            cout << itMap->first << "\t" << itMap->second << "|";
        }
        cout << endl;
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
            else if (itMap->second > max)
            {
                max = itMap->second;
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

    for(std::vector<Star>::const_iterator it = starList.begin(); it != starList.end(); ++it)
    {
        cout << it->first << "\t" << it->second << endl;
    }

}

std::vector<int> StarIdentifier::identifyPyramidMethod(const StarIdentifier::vectorList_t &starVectors, float eps)
{
    if(!mOpenDb)
        throw std::runtime_error("No Database opened");

    // Vector which holds a map with possible hip for each spot
    typedef std::vector<std::map<int, int> > idTable_t ;
    idTable_t idTable(starVectors.size(), std::map<int,int>() );


    // prepare a statement for the database which searches for the feature/angle within an interval
    const std::string sql("SELECT * FROM featureList WHERE theta >? AND theta < ?");
    sqlite3_stmt * sqlStmt;
    if (sqlite3_prepare_v2(mDb, sql.c_str(), sql.size()+1, &sqlStmt, 0) != SQLITE_OK)
        throw std::runtime_error("Prparing SQL search query failed");


    /* Algorithm:
     *  1. Take 3 stars (take them in variable order)
     *  2. Calculate 3 angles between them
     *  3. Search feature list for angles from 2.
     *  4. Try to find a unique triangle in the results
     *      - On success:
     *          5. Take 4th star
     *          6. Calculate the 3 new angles between 4th star and 3 identified stars
     *          7. Try to find the 4th star in the catalog
     *          - On success:
     *              - Identification successful, do 5 to 7 for all remaining stars
     *                in order to identify all remaining true and false stars
     *              - On failure:
     *              - Goto 5.
     *       - On failure:
     *          - Goto 1
     */
    int nSpots = starVectors.size();
    if( nSpots < 4)
        throw std::range_error("At least 4 star spots necessary");

    std::vector<int> idList;

    // Stop iteration as soon as one unique triad is identified
    bool identificationComplete = false;

    // iteration in the order suggested by Mortari 2004
    for(int dj=1; dj<(nSpots-1) && !identificationComplete; ++dj)
    {
        for(int dk=1; dk<(nSpots-dj) && !identificationComplete; ++dk)
        {
            for(int i=0; i<(nSpots-dj-dk) && !identificationComplete; ++i)
            {
                int j = i + dj;
                int k = j + dk;
                idList.assign(nSpots, -1);

                // calculate the 3 angles (features)
                const float RAD_TO_DEG = 180 / M_PI;
                // first dot product between each 2 vectors
                float thetaIJ = starVectors[i].dot(starVectors[j]) / (starVectors[i].norm() * starVectors[j].norm() );
                float thetaIK = starVectors[i].dot(starVectors[k]) / (starVectors[i].norm() * starVectors[k].norm() );
                float thetaJK = starVectors[j].dot(starVectors[k]) / (starVectors[j].norm() * starVectors[k].norm() );

                // arccos and conversion to deg
                thetaIJ = acos(thetaIJ) * RAD_TO_DEG;
                thetaIK = acos(thetaIK) * RAD_TO_DEG;
                thetaJK = acos(thetaJK) * RAD_TO_DEG;

                // get a list with possible candidates for each theta
                featureList_t listIJ, listIK, listJK;

                // bind upper and lower bound to the sql statement
                if(sqlite3_reset(sqlStmt) != SQLITE_OK) throw std::runtime_error("Resetting SQL query failed");
                if(sqlite3_bind_double(sqlStmt, 1, thetaIJ - eps) != SQLITE_OK) throw std::runtime_error("Binding new value1 to query failed");
                if(sqlite3_bind_double(sqlStmt, 2, thetaIJ + eps) != SQLITE_OK) throw std::runtime_error("Binding new value2 to query failed");
                retrieveFeatureList(sqlStmt, listIJ);
                // if list is empty skip further processing
                if(listIJ.empty() ) continue;

                // bind upper and lower bound to the sql statement
                if(sqlite3_reset(sqlStmt) != SQLITE_OK) throw std::runtime_error("Resetting SQL query failed");
                if(sqlite3_bind_double(sqlStmt, 1, thetaIK - eps) != SQLITE_OK) throw std::runtime_error("Binding new value1 to query failed");
                if(sqlite3_bind_double(sqlStmt, 2, thetaIK + eps) != SQLITE_OK) throw std::runtime_error("Binding new value2 to query failed");
                retrieveFeatureList(sqlStmt, listIK);
                // if list is empty skip further processing
                if(listIK.empty() ) continue;

                // bind upper and lower bound to the sql statement
                if(sqlite3_reset(sqlStmt) != SQLITE_OK) throw std::runtime_error("Resetting SQL query failed");
                if(sqlite3_bind_double(sqlStmt, 1, thetaJK - eps) != SQLITE_OK) throw std::runtime_error("Binding new value1 to query failed");
                if(sqlite3_bind_double(sqlStmt, 2, thetaJK + eps) != SQLITE_OK) throw std::runtime_error("Binding new value2 to query failed");
                retrieveFeatureList(sqlStmt, listJK);
                // if list is empty skip further processing
                if(listJK.empty() ) continue;

                // find possible triads

                int hipI, hipJ, hipK, count = 0;
                for(featureList_t::const_iterator itIJ = listIJ.begin(), endIJ = listIJ.end(); itIJ != endIJ; ++itIJ)
                {
                    int tempI, tempJ, tempK;
                    for(featureList_t::const_iterator itIK = listIK.begin(), endIK = listIK.end(); itIK != endIK; ++itIK)
                    {
                        if(itIJ->id1 == itIK->id1 || itIJ->id2 == itIK->id1)
                        {
                            tempI = itIK->id1;
                            tempJ = (itIJ->id1 == tempI) ? itIJ->id2 : itIJ->id1;
                            tempK = itIK->id2;
                        }
                        else if(itIJ->id1 == itIK->id2 || itIJ->id2 == itIK->id2)
                        {
                            tempI = itIK->id2;
                            tempJ = (itIJ->id1 == tempI) ? itIJ->id2 : itIJ->id1;
                            tempK = itIK->id1;
                        }
                        else
                        {
                            continue;
                        }

                        for(featureList_t::const_iterator itJK = listJK.begin(), endJK = listJK.end(); itJK != endJK; ++itJK)
                        {
                            if(itJK->id1 == tempK || itJK->id2 == tempK)
                            {
                                if(itJK->id1 == tempJ || itJK->id2 == tempJ)
                                {
                                    hipI = tempI;
                                    hipJ = tempJ;
                                    hipK = tempK;
                                    count++;
                                    break;
                                }
                            }

                        }
                    }
                }

                // if no unique triangle was found try next triad
                if(count != 1)
                {
                    continue;
                }

                idList[i] = hipI;
                idList[j] = hipJ;
                idList[k] = hipK;
                // check if a matching 4th star is found and if identify all remaining spots
                for(int r=0; r<nSpots; ++r)
                {
                    // ignore the stars of the triad
                    if((r == i) || (r == j) || (r == k))
                        continue;

                    // calculate the angles between the new 4th star and the stars of the triad
                    // first dot product between each 2 vectors
                    float thetaIR = starVectors[i].dot(starVectors[r]) / (starVectors[i].norm() * starVectors[r].norm() );
                    float thetaJR = starVectors[j].dot(starVectors[r]) / (starVectors[j].norm() * starVectors[r].norm() );
                    float thetaKR = starVectors[k].dot(starVectors[r]) / (starVectors[k].norm() * starVectors[r].norm() );

                    // arccos and conversion to deg
                    thetaIR = acos(thetaIR) * RAD_TO_DEG;
                    thetaJR = acos(thetaJR) * RAD_TO_DEG;
                    thetaKR = acos(thetaKR) * RAD_TO_DEG;

                    // search in the catalog for the 4th star
                    featureList_t listIR, listJR, listKR;

                    // prepare a statement for the database which searches for the feature/angle within an interval and for the correct hip
                    const std::string sql("SELECT * FROM featureList WHERE (theta >? AND theta < ?) AND (hip1 = ? OR hip2 = ?)");
                    sqlite3_stmt * sqlFinal;
                    if (sqlite3_prepare_v2(mDb, sql.c_str(), sql.size()+1, &sqlFinal, 0) != SQLITE_OK) throw std::runtime_error("Prparing SQL search query failed");

                    if(sqlite3_reset(sqlFinal) != SQLITE_OK) throw std::runtime_error("Resetting SQL query failed");
                    if(sqlite3_bind_double(sqlFinal, 1, thetaIR - eps) != SQLITE_OK) throw std::runtime_error("Binding new value1 to query failed");
                    if(sqlite3_bind_double(sqlFinal, 2, thetaIR + eps) != SQLITE_OK) throw std::runtime_error("Binding new value2 to query failed");
                    if(sqlite3_bind_int   (sqlFinal, 3, hipI)          != SQLITE_OK) throw std::runtime_error("Binding new value3 to query failed");
                    if(sqlite3_bind_int   (sqlFinal, 4, hipI)          != SQLITE_OK) throw std::runtime_error("Binding new value4 to query failed");
                    retrieveFeatureList(sqlFinal, listIR);
                    // if list is empty skip further processing
                    if(listIR.empty() ) continue;

                    if(sqlite3_reset(sqlFinal) != SQLITE_OK) throw std::runtime_error("Resetting SQL query failed");
                    if(sqlite3_bind_double(sqlFinal, 1, thetaJR - eps) != SQLITE_OK) throw std::runtime_error("Binding new value1 to query failed");
                    if(sqlite3_bind_double(sqlFinal, 2, thetaJR + eps) != SQLITE_OK) throw std::runtime_error("Binding new value2 to query failed");
                    if(sqlite3_bind_int   (sqlFinal, 3, hipJ)          != SQLITE_OK) throw std::runtime_error("Binding new value3 to query failed");
                    if(sqlite3_bind_int   (sqlFinal, 4, hipJ)          != SQLITE_OK) throw std::runtime_error("Binding new value4 to query failed");
                    retrieveFeatureList(sqlFinal, listJR);
                    // if list is empty skip further processing
                    if(listJR.empty() ) continue;

                    if(sqlite3_reset(sqlFinal) != SQLITE_OK) throw std::runtime_error("Resetting SQL query failed");
                    if(sqlite3_bind_double(sqlFinal, 1, thetaKR - eps) != SQLITE_OK) throw std::runtime_error("Binding new value1 to query failed");
                    if(sqlite3_bind_double(sqlFinal, 2, thetaKR + eps) != SQLITE_OK) throw std::runtime_error("Binding new value2 to query failed");
                    if(sqlite3_bind_int   (sqlFinal, 3, hipK)          != SQLITE_OK) throw std::runtime_error("Binding new value3 to query failed");
                    if(sqlite3_bind_int   (sqlFinal, 4, hipK)          != SQLITE_OK) throw std::runtime_error("Binding new value4 to query failed");
                    retrieveFeatureList(sqlFinal, listKR);
                    // if list is empty skip further processing
                    if(listKR.empty() ) continue;


                    // check for a unique solution
                    /// TODO: is there a more elegant solution completely in sql?
                    count = 0;
                    int idCheck;
                    for(featureList_t::const_iterator itIR=listIR.begin(), endIR = listIR.end(); itIR != endIR; ++itIR)
                    {
                        idCheck = (itIR->id1 == hipI) ? itIR->id2 : itIR->id1;

                        for(featureList_t::const_iterator itJR=listJR.begin(), endJR = listJR.end(); itJR != endJR; ++itJR)
                        {
                            if(itJR->id1 == idCheck || itJR->id2 == idCheck)
                            {
                                for(featureList_t::const_iterator itKR=listKR.begin(), endKR = listKR.end(); itKR != endKR; ++itKR)
                                {
                                    if(itKR->id1 == idCheck || itKR->id2 == idCheck)
                                    {
                                        count++;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // if count == 1, everything is good
                    if(count == 1)
                    {
                        idList[r] = idCheck;

                        // at least one 4th star found therefore the triad is confirmed and
                        // after the current loop (with r) is through the identification is completed
                        identificationComplete = true;
                        cout << "success" << endl;
                    }
                }
            }
        }
    }
    return idList;
}

std::vector<int> StarIdentifier::identifyPyramidMethodKVector(const StarIdentifier::vectorList_t &starVectors, float eps)
{

    if(mKVector.empty() || mFeatureList.empty() )
        throw std::runtime_error("No feature list loaded");

    // Vector which holds a map with possible hip for each spot
    typedef std::vector<std::map<int, int> > idTable_t ;
    idTable_t idTable(starVectors.size(), std::map<int,int>() );

    /* Algorithm:
     *  1. Take 3 stars (take them in variable order)
     *  2. Calculate 3 angles between them
     *  3. Search feature list for angles from 2.
     *  4. Try to find a unique triangle in the results
     *      - On success:
     *          5. Take 4th star
     *          6. Calculate the 3 new angles between 4th star and 3 identified stars
     *          7. Try to find the 4th star in the catalog
     *          - On success:
     *              - Identification successful, do 5 to 7 for all remaining stars
     *                in order to identify all remaining true and false stars
     *              - On failure:
     *              - Goto 5.
     *       - On failure:
     *          - Goto 1
     */
    int nSpots = starVectors.size();
    if( nSpots < 4)
        throw std::range_error("At least 4 star spots necessary");

    std::vector<int> idList;

    // Stop iteration as soon as one unique triad is identified
    bool identificationComplete = false;

    // iteration in the order suggested by Mortari 2004
    for(int dj=1; dj<(nSpots-1) && !identificationComplete; ++dj)
    {
        for(int dk=1; dk<(nSpots-dj) && !identificationComplete; ++dk)
        {
            for(int i=0; i<(nSpots-dj-dk) && !identificationComplete; ++i)
            {
                int j = i + dj;
                int k = j + dk;
                idList.assign(nSpots, -1);

                // calculate the 3 angles (features)
                const float RAD_TO_DEG = 180 / M_PI;
                // first dot product between each 2 vectors
                float thetaIJ = starVectors[i].dot(starVectors[j]) / (starVectors[i].norm() * starVectors[j].norm() );
                float thetaIK = starVectors[i].dot(starVectors[k]) / (starVectors[i].norm() * starVectors[k].norm() );
                float thetaJK = starVectors[j].dot(starVectors[k]) / (starVectors[j].norm() * starVectors[k].norm() );

                // arccos and conversion to deg
                thetaIJ = acos(thetaIJ) * RAD_TO_DEG;
                thetaIK = acos(thetaIK) * RAD_TO_DEG;
                thetaJK = acos(thetaJK) * RAD_TO_DEG;

                // get a list with possible candidates for each theta
                featureList_t listIJ, listIK, listJK;

                retrieveFeatureListKVector(thetaIJ - eps, thetaIJ + eps, listIJ);
                // if list is empty skip further processing
                if(listIJ.empty() ) continue;

                retrieveFeatureListKVector(thetaIK - eps, thetaIK + eps, listIK);
                // if list is empty skip further processing
                if(listIK.empty() ) continue;

                retrieveFeatureListKVector(thetaJK - eps, thetaJK + eps, listJK);
                // if list is empty skip further processing
                if(listJK.empty() ) continue;

                // find possible triads

                int hipI, hipJ, hipK, count = 0;
                for(featureList_t::const_iterator itIJ = listIJ.begin(), endIJ = listIJ.end(); itIJ != endIJ; ++itIJ)
                {
                    int tempI, tempJ, tempK;
                    for(featureList_t::const_iterator itIK = listIK.begin(), endIK = listIK.end(); itIK != endIK; ++itIK)
                    {
                        if(itIJ->id1 == itIK->id1 || itIJ->id2 == itIK->id1)
                        {
                            tempI = itIK->id1;
                            tempJ = (itIJ->id1 == tempI) ? itIJ->id2 : itIJ->id1;
                            tempK = itIK->id2;
                        }
                        else if(itIJ->id1 == itIK->id2 || itIJ->id2 == itIK->id2)
                        {
                            tempI = itIK->id2;
                            tempJ = (itIJ->id1 == tempI) ? itIJ->id2 : itIJ->id1;
                            tempK = itIK->id1;
                        }
                        else
                        {
                            continue;
                        }

                        for(featureList_t::const_iterator itJK = listJK.begin(), endJK = listJK.end(); itJK != endJK; ++itJK)
                        {
                            if(itJK->id1 == tempK || itJK->id2 == tempK)
                            {
                                if(itJK->id1 == tempJ || itJK->id2 == tempJ)
                                {
                                    hipI = tempI;
                                    hipJ = tempJ;
                                    hipK = tempK;
                                    count++;
                                    break;
                                }
                            }

                        }
                    }
                }

                // if no unique triangle was found try next triad
                if(count != 1)
                {
                    continue;
                }

                idList[i] = hipI;
                idList[j] = hipJ;
                idList[k] = hipK;
                // check if a matching 4th star is found and if identify all remaining spots
                for(int r=0; r<nSpots; ++r)
                {
                    // ignore the stars of the triad
                    if((r == i) || (r == j) || (r == k))
                        continue;

                    // calculate the angles between the new 4th star and the stars of the triad
                    // first dot product between each 2 vectors
                    float thetaIR = starVectors[i].dot(starVectors[r]) / (starVectors[i].norm() * starVectors[r].norm() );
                    float thetaJR = starVectors[j].dot(starVectors[r]) / (starVectors[j].norm() * starVectors[r].norm() );
                    float thetaKR = starVectors[k].dot(starVectors[r]) / (starVectors[k].norm() * starVectors[r].norm() );

                    // arccos and conversion to deg
                    thetaIR = acos(thetaIR) * RAD_TO_DEG;
                    thetaJR = acos(thetaJR) * RAD_TO_DEG;
                    thetaKR = acos(thetaKR) * RAD_TO_DEG;

                    // search in the catalog for the 4th star
                    featureList_t listIR, listJR, listKR;

                    retrieveFeatureListKVector(thetaIR - eps, thetaIR + eps, hipI, listIR);
                    // if list is empty skip further processing
                    if(listIR.empty() ) continue;

                    retrieveFeatureListKVector(thetaJR - eps, thetaJR + eps, hipJ, listJR);
                    // if list is empty skip further processing
                    if(listJR.empty() ) continue;

                    retrieveFeatureListKVector(thetaKR - eps, thetaKR + eps, hipK, listKR);
                    // if list is empty skip further processing
                    if(listKR.empty() ) continue;

                    // check for a unique solution
                    /// TODO: is there a more elegant solution completely in sql?
                    count = 0;
                    int idCheck;
                    for(featureList_t::const_iterator itIR=listIR.begin(), endIR = listIR.end(); itIR != endIR; ++itIR)
                    {
                        idCheck = (itIR->id1 == hipI) ? itIR->id2 : itIR->id1;

                        for(featureList_t::const_iterator itJR=listJR.begin(), endJR = listJR.end(); itJR != endJR; ++itJR)
                        {
                            if(itJR->id1 == idCheck || itJR->id2 == idCheck)
                            {
                                for(featureList_t::const_iterator itKR=listKR.begin(), endKR = listKR.end(); itKR != endKR; ++itKR)
                                {
                                    if(itKR->id1 == idCheck || itKR->id2 == idCheck)
                                    {
                                        count++;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // if count == 1, everything is good
                    if(count == 1)
                    {
                        idList[r] = idCheck;

                        // at least one 4th star found therefore the triad is confirmed and
                        // after the current loop (with r) is through the identification is completed
                        identificationComplete = true;
                    }
                }
            }
        }
    }
    return idList;
}

void StarIdentifier::createFeatureList2(const vectorList_t &starVectors, featureList_t &output) const
{
    // start from the beginning
    output.clear();

    const float RAD_TO_DEG = 180 / M_PI;

    vectorList_t::const_iterator it1, end1, it2;
    end1 = starVectors.end() -1 ;

    /* NOTE: i and j are only for counting purposes.
     *       they also could be used for direct accessing the vector elements
     *       but iterators clutter the code less  */
    int i = 0;
    for(it1 = starVectors.begin(); it1 != end1; ++it1, ++i)
    {
        int j = i+1;
        for( it2 = it1+1; it2!= starVectors.end(); ++it2, ++j)
        {
            float dot = it1->dot(*it2) / (it1->norm() * it2->norm()) ;
            // compute the angle between the two vectors and add to the feature list
            float theta = acos(dot ) * RAD_TO_DEG;
            output.push_back(Feature2(i, j, theta) ) ;
        }
    }

    for(featureList_t::const_iterator it = output.begin(); it != output.end(); ++it)
    {
        cout << it->id1 << "\t" << it->id2 << "\t" << it->theta << endl;
    }
}

void StarIdentifier::retrieveFeatureList(sqlite3_stmt * sqlStmt, StarIdentifier::featureList_t &output) const
{
    output.clear();

    int result;
    while( (result = sqlite3_step(sqlStmt)) == SQLITE_ROW)
    {
        // get the first hip
        int hip1 = sqlite3_column_int(sqlStmt, 0);
        int hip2 = sqlite3_column_int(sqlStmt, 1);
        float theta = sqlite3_column_double(sqlStmt, 2);

        output.push_back(Feature2(hip1, hip2, theta));
    }

    if (result != SQLITE_DONE)
        throw std::runtime_error("SQL search returned with unexpected result");
}

void StarIdentifier::retrieveFeatureListKVector(float thetaMin, float thetaMax, StarIdentifier::featureList_t &output) const
{

    output.clear();
    // caclulate k-indices (jb and jt in mortari)
    unsigned int jb = (unsigned int) ((thetaMin -mQ) / mM);
    unsigned int jt = (unsigned int) ((thetaMax -mQ) / mM) +1; //always round up

    // calculate get bottom and to index from the kVector
    unsigned int kb = mKVector[jb] + 1;
    unsigned int kt = mKVector[jt];

    for(unsigned int i=kb; i<=kt; ++i)
    {
        output.push_back(mFeatureList[i]);
    }
}

void StarIdentifier::retrieveFeatureListKVector(float thetaMin, float thetaMax, int hip, StarIdentifier::featureList_t &output) const
{
    output.clear();
    // caclulate k-indices (jb and jt in mortari)
    unsigned int jb = (unsigned int) ((thetaMin -mQ) / mM);
    unsigned int jt = (unsigned int) ((thetaMax -mQ) / mM) +1; //always round up

    // calculate get bottom and to index from the kVector
    unsigned int kb = mKVector[jb] + 1;
    unsigned int kt = mKVector[jt];

    for(unsigned int i=kb; i<=kt; ++i)
    {
        Feature2 temp = mFeatureList[i];
        if (temp.id1 == hip || temp.id2 == hip)
        {
            output.push_back(temp);
        }
    }
}




