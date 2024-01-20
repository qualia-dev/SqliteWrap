#ifndef SQLITEWRAP_H
#define SQLITEWRAP_H

#include <string>
#include <vector>
#include <memory>

#include "SqliteWrap_global.h"
#include "sqlite3.h"

using DeserializeCallback = bool (*)(void*, char**, int);

class SQLITEWRAP_EXPORT SqliteWrap
{
public:
    SqliteWrap();

    bool is_connected() const { return _db != nullptr; }

    bool connect(const std::string& db_name);
    void connect_(const std::string& db_name);   // version with try catch throw
    bool disconnect();
    bool disconnect_();                          // version with try catch throw
    bool exists(const std::string& db_name);
    bool create_db(const std::string& db_name);
    bool delete_db(const std::string& db_name);

    bool execute_sql(const std::string& sql);

    // select command
    bool select_count_sync (const std::string &table, const std::string &condition, int &count);
    bool select(const std::string &table, const std::string &condition, void* user_param, int (*callback)(void*,int,char**,char**), int &count);

    bool select_sync(const std::string &table, const std::string &condition, void* user_param, DeserializeCallback callback );

    bool get_sqlite_version (std::string &version);
    bool get_database_name (std::string &db_name);
    bool get_table_list(std::vector<std::string>& table_list);

    bool get_database_schema(const std::string& pathfile);
    bool execute_sql_schema(const std::string& pathfile);

    bool get_table_content(const std::string &table_name, std::vector<std::vector<std::tuple<std::unique_ptr<std::string>, std::unique_ptr<std::string>, std::unique_ptr<std::string>>>> &table_content);

private:
    sqlite3* _db = nullptr;
    std::string _last_error;

public:
    // getter
    const std::string& get_last_error() const { return _last_error; }
};

#endif // SQLITEWRAP_H
