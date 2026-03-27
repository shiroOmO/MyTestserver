#include "SessionManager.h"
#include "Logger.h"
#include <algorithm>

SessionManager::SessionManager() {
}

SessionManager::~SessionManager() {
}

void SessionManager::addSession(const std::string& username, const TcpConnectionPtr& conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    Session session;
    session.conn = conn;
    session.lastHeartbeat = Timestamp::now();
    sessions_[username] = session;
    LOG_INFO("User %s logged in, total online: %zu", username.c_str(), sessions_.size());
}

void SessionManager::removeSession(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(username);
    LOG_INFO("User %s logged out, total online: %zu", username.c_str(), sessions_.size());
}

TcpConnectionPtr SessionManager::getConnection(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(username);
    if (it != sessions_.end()) {
        return it->second.conn;
    }
    return nullptr;
}

std::vector<std::string> SessionManager::getOnlineUsers() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> users;
    users.reserve(sessions_.size());
    for (auto& pair : sessions_) {
        users.push_back(pair.first);
    }
    return users;
}

bool SessionManager::isOnline(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.find(username) != sessions_.end();
}

void SessionManager::updateHeartbeat(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(username);
    if (it != sessions_.end()) {
        it->second.lastHeartbeat = Timestamp::now();
    }
}

void SessionManager::broadcastToAll(const std::string& message, const std::string& excludeUsername) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : sessions_) {
        if (pair.first != excludeUsername && pair.second.conn->connected()) {
            pair.second.conn->send(message);
        }
    }
}
