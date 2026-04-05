#include "ChatService.h"
#include "Protocol.h"
#include "Logger.h"
#include "json.hpp"
#include "WebSocketHandler.h"

#include <chrono>


using json = nlohmann::json;

ChatService::ChatService() {
    userDao_.reset(new UserDao());
    messageDao_.reset(new MessageDao());
    sessionManager_.reset(new SessionManager());
}

ChatService::~ChatService() {
}

void ChatService::sendResponse(const TcpConnectionPtr &conn, const std::string &json) {
    std::string frame = WebSocketHandler::encodeFrame(json);
    conn->send(frame);
}

std::string ChatService::createResponse(const std::string &msgType, bool success, const std::string &error) {
    json j;
    j[FIELD_MSG_TYPE] = msgType;
    j[FIELD_SUCCESS] = success;
    if(!error.empty()) {
        j[FIELD_ERROR] = error;
    }
    return j.dump();
}

void ChatService::handleRegister(const std::string &username, const std::string &password,
        const TcpConnectionPtr &conn, const std::string &connectionUsername) {

    if(username.empty() || password.empty()) {
        sendResponse(conn, createResponse(MSG_TYPE_REGISTER_RESP, false, "Username and password cannot be empty"));
        return;
    }

    if(userDao_->userExists(username)) {
        sendResponse(conn, createResponse(MSG_TYPE_REGISTER_RESP, false, "Username already exists"));
        return;
    }

    bool success = userDao_->insertUser(username, password);
    if(success) {
        LOG_INFO("User %s registered successfully", username.c_str());
        sendResponse(conn, createResponse(MSG_TYPE_REGISTER_RESP, true));
    }
    else {
        sendResponse(conn, createResponse(MSG_TYPE_REGISTER_RESP, false, "Registration failed"));
    }
}

void ChatService::handleLogin(const std::string &username, const std::string &password, const TcpConnectionPtr& conn) {
    if(username.empty() || password.empty()) {
        sendResponse(conn, createResponse(MSG_TYPE_LOGIN_RESP, false, "Username and password cannot be empty"));
        return;
    }

    if(!userDao_->verifyUser(username, password)) {
        sendResponse(conn, createResponse(MSG_TYPE_LOGIN_RESP, false, "Invalid username or password"));
        return;
    }

    if(sessionManager_->isOnline(username)) {
        sendResponse(conn, createResponse(MSG_TYPE_LOGIN_RESP, false, "User already logged in"));
        return;
    }

    auto user = userDao_->getUserByUsername(username);
    sessionManager_->addSession(username, conn);

    json response;
    response[FIELD_MSG_TYPE] = MSG_TYPE_LOGIN_RESP;
    response[FIELD_SUCCESS] = true;
    response[FIELD_ONLINE_USERS] = sessionManager_->getOnlineUsers();
    sendResponse(conn, response.dump());

    LOG_INFO("User %s logged in successfully", username.c_str());
}

void ChatService::handleLogout(const std::string &username, const TcpConnectionPtr &conn) {
    sessionManager_->removeSession(username);
    json response;
    response[FIELD_MSG_TYPE] = MSG_TYPE_LOGOUT_RESP;
    response[FIELD_SUCCESS] = true;
    sendResponse(conn, response.dump());
    conn->shutdown();
    LOG_INFO("User %s logged out", username.c_str());
}

void ChatService::handleChat(const std::string &sender, const std::string &content) {
    if(content.empty() || !sessionManager_->isOnline(sender)) {
        return;
    }

    auto user = userDao_->getUserByUsername(sender);
    if(!user) {
        return;
    }

    // Save to database
    messageDao_->saveMessage(user->id, content);

    // Broadcast to all online users
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    json broadcast;
    broadcast[FIELD_MSG_TYPE] = MSG_TYPE_BROADCAST;
    broadcast[FIELD_SENDER] = sender;
    broadcast[FIELD_CONTENT] = content;
    broadcast[FIELD_TIMESTAMP] = timestamp;

    std::string broadcastStr = broadcast.dump();
    std::string frame = WebSocketHandler::encodeFrame(broadcastStr);
    sessionManager_->broadcastToAll(frame, "");
    LOG_INFO("Broadcast message from %s: %zu chars", sender.c_str(), content.size());
}

void ChatService::handleHeartbeat(const std::string &username) {
    sessionManager_->updateHeartbeat(username);
    auto now = std::chrono::system_clock::now();
    auto serverTime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    json response;
    response[FIELD_MSG_TYPE] = MSG_TYPE_HEARTBEAT_RESP;
    response[FIELD_SERVER_TIME] = serverTime;

    auto conn = sessionManager_->getConnection(username);
    if(conn && conn->connected()) {
        sendResponse(conn, response.dump());
    }
}

void ChatService::handleGetHistory(const TcpConnectionPtr &conn, int limit) {
    auto messages = messageDao_->getRecentMessages(limit);
    json response;
    response[FIELD_MSG_TYPE] = MSG_TYPE_HISTORY;
    response[FIELD_MESSAGES] = json::array();
    for(auto &msg : messages) {
        json jmsg;
        jmsg[FIELD_SENDER] = msg.username;
        jmsg[FIELD_CONTENT] = msg.content;
        jmsg[FIELD_TIMESTAMP] = msg.timestamp;
        response[FIELD_MESSAGES].push_back(jmsg);
    }
    sendResponse(conn, response.dump());
}

void ChatService::onDisconnect(const std::string &username) {
    if (!username.empty() && sessionManager_->isOnline(username)) {
        sessionManager_->removeSession(username);
    }
}

void ChatService::keepMySQLAlive() {
    if(!userDao_->heartBeat())
        LOG_FATAL("keepMySQLAlive Fatal! userDao_ disconnection. ");
    if(!messageDao_->heartBeat())
        LOG_FATAL("keepMySQLAlive Fatal! messageDao_ disconnection. ");
    LOG_INFO("keepMySQLAlive success.");
}


