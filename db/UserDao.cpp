#include "UserDao.h"


UserDao::UserDao() : connection_() {
}

UserDao::~UserDao() {
}

bool UserDao::insertUser(const std::string &username, const std::string &password) {
    if (!connection_.connected()) {
        return false;
    }

    std::string escapedUsername = connection_.escape(username);
    std::string escapedPassword = connection_.escape(password);

    std::string sql = "INSERT INTO users (username, password) VALUES ('" +
                      escapedUsername + "', '" + escapedPassword + "')";

    return connection_.execute(sql);
}

bool UserDao::verifyUser(const std::string &username, const std::string &password) {
    auto user = getUserByUsername(username);
    if (!user) {
        return false;
    }
    return user->password == password;
}

std::shared_ptr<User> UserDao::getUserByUsername(const std::string &username) {
    if (!connection_.connected()) {
        return nullptr;
    }

    std::string escapedUsername = connection_.escape(username);
    std::string sql = "SELECT id, username, password FROM users WHERE username = '" + escapedUsername + "'";

    MYSQL_RES *result = connection_.query(sql);
    if (!result) {
        return nullptr;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return nullptr;
    }

    auto user = std::make_shared<User>();
    user->id = atoi(row[0]);
    user->username = row[1] ? row[1] : "";
    user->password = row[2] ? row[2] : "";

    mysql_free_result(result);
    return user;
}

bool UserDao::userExists(const std::string &username) {
    return getUserByUsername(username) != nullptr;
}

bool UserDao::heartBeat() {
    if (!connection_.connected()) {
        return false;
    }

    std::string sql = "SELECT 1";
    MYSQL_RES *result = connection_.query(sql);

    return result ? true : false;
}

