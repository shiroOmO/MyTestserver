#pragma once

#include "UserDao.h"
#include "MessageDao.h"
#include "SessionManager.h"

#include <memory>
#include <string>

class ChatService {
public:
    ChatService();
    ~ChatService();

    void handleRegister(const std::string &username, const std::string &password, const TcpConnectionPtr &conn, const std::string &connectionUsername);
    void handleLogin(const std::string &username, const std::string &password, const TcpConnectionPtr &conn);
    void handleLogout(const std::string &username, const TcpConnectionPtr &conn);
    void handleChat(const std::string &sender, const std::string &content);
    void handleHeartbeat(const std::string &username);
    void handleGetHistory(const TcpConnectionPtr &conn, int limit);
    void onDisconnect(const std::string &username);
    void keepMySQLAlive();

private:
    void sendResponse(const TcpConnectionPtr &conn, const std::string &json);
    std::string createResponse(const std::string &msgType, bool success, const std::string &error = "");

private:
    std::unique_ptr<UserDao> userDao_;
    std::unique_ptr<MessageDao> messageDao_;
    std::unique_ptr<SessionManager> sessionManager_;
};

