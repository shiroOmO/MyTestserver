#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "TcpConnection.h"
#include "Timestamp.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

class SessionManager {
public:
    SessionManager();
    ~SessionManager();

    void addSession(const std::string& username, const TcpConnectionPtr& conn);
    void removeSession(const std::string& username);
    TcpConnectionPtr getConnection(const std::string& username);
    std::vector<std::string> getOnlineUsers();
    bool isOnline(const std::string& username);
    void updateHeartbeat(const std::string& username);
    void broadcastToAll(const std::string& message, const std::string& excludeUsername = "");

private:
    struct Session {
        TcpConnectionPtr conn;
        Timestamp lastHeartbeat;
    };

    std::unordered_map<std::string, Session> sessions_;
    std::mutex mutex_;
};

#endif // SESSIONMANAGER_H
