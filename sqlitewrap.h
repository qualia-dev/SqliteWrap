#ifndef SQLITEWRAP_H
#define SQLITEWRAP_H

#include <string>

#include "SqliteWrap_global.h"
#include "sqlite3.h"

class SQLITEWRAP_EXPORT SqliteWrap
{
public:
    SqliteWrap();

    bool connect(const std::string& db_name);
    void connect_(const std::string& db_name);   // version with try catch throw
    bool exists(const std::string& db_name);
    bool create_db(const std::string& db_name);
    bool delete_db(const std::string& db_name);

private:
    sqlite3* db = nullptr;
};

#endif // SQLITEWRAP_H
