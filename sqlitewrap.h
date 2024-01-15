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

    bool execute_sql(const std::string& sql);

    // select command
    bool select_count (const std::string &table, const std::string &condition, int &count);
    bool select_count_async (const std::string &table, const std::string &condition, int &count);
    bool select(const std::string &table, const std::string &condition, void* user_param, int (*callback)(void*,int,char**,char**), std::string &result);

private:
    sqlite3* _db = nullptr;
    std::string _last_error;

public:
    // getter
    const std::string& get_last_error() const { return _last_error; }
};

#endif // SQLITEWRAP_H
