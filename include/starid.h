#ifndef STARID_H
#define STARID_H

#include<sqlite3.h>

#include "datatypes.h"


class StarIdentifier
{
    StarIdentifier();

    ~StarIdentifier();

    void setFeatureListDB(const char* filename);

    bool openDb();

private:
    char* mDbFile;
    sqlite3 * mDb;
};

#endif // STARCAMERA_H
