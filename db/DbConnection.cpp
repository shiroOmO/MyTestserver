#include "DbConnection.h"
#include "Logger.h"

DbConnection::DbConnection() : connected_(false) {
    mysql_ = mysql_init(nullptr);
    if (!mysql_) {
        LOG_ERROR("Failed to initialize MySQL");
        return;
    }

    // Connect using provided credentials
    if (!mysql_real_connect(mysql_, "localhost", "mytestserver_user", "123456", "mytestserver", 0, nullptr, 0)) {
        LOG_ERROR("Failed to connect to MySQL: %s", mysql_error(mysql_));
        mysql_close(mysql_);
        return;
    }

    connected_ = true;
    LOG_INFO("Connected to MySQL database successfully");
}

DbConnection::~DbConnection() {
    if (connected_) {
        mysql_close(mysql_);
        connected_ = false;
    }
}

bool DbConnection::connected() const {
    return connected_;
}

bool DbConnection::execute(const std::string& sql) {
    if (!connected_) {
        return false;
    }
    if (mysql_query(mysql_, sql.c_str()) != 0) {
        LOG_ERROR("MySQL execute failed: %s", mysql_error(mysql_));
        return false;
    }
    return true;
}

MYSQL_RES* DbConnection::query(const std::string& sql) {
    if (!connected_) {
        return nullptr;
    }
    if (mysql_query(mysql_, sql.c_str()) != 0) {
        LOG_ERROR("MySQL query failed: %s", mysql_error(mysql_));
        return nullptr;
    }
    return mysql_store_result(mysql_);
}

std::string DbConnection::escape(const std::string& str) {
    char* buf = new char[str.size() * 2 + 1];
    mysql_real_escape_string(mysql_, buf, str.c_str(), str.size());
    std::string result(buf);
    delete[] buf;
    return result;
}
