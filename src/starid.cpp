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

