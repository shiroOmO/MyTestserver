#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <mysql/mysql.h>
#include <string>

class DbConnection {
public:
    DbConnection();
    ~DbConnection();

    bool connected() const;
    bool execute(const std::string& sql);
    MYSQL_RES* query(const std::string& sql);
    std::string escape(const std::string& str);

private:
    MYSQL* mysql_;
    bool connected_;
};

#endif // DBCONNECTION_H
