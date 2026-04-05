#pragma once

#include "DbConnection.h"

#include <string>
#include <memory>


struct User {
    int id;
    std::string username;
    std::string password;
};

class UserDao {
public:
    UserDao();
    ~UserDao();

    bool insertUser(const std::string &username, const std::string &password);
    bool verifyUser(const std::string &username, const std::string &password);
    std::shared_ptr<User> getUserByUsername(const std::string &username);
    bool userExists(const std::string &username);
    bool heartBeat();

private:
    DbConnection connection_;
};

