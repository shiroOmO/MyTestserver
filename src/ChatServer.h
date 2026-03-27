#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "ChatService.h"
#include "WebSocketHandler.h"
#include <memory>
#include <string>
#include <unordered_map>

class ChatServer {
public:
    ChatServer(EventLoop* loop, const InetAddress& addr, const std::string& name);
    ~ChatServer();

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
    bool handleHttpRequest(const TcpConnectionPtr& conn, const std::string& request);
    void handleWebSocketMessage(const TcpConnectionPtr& conn, const std::string& payload);
    std::string getCurrentUsername(const TcpConnectionPtr& conn);

private:
    struct ConnectionState {
        bool isWebSocket;
        bool handshakeComplete;
        std::string username;
    };

    TcpServer server_;
    EventLoop* loop_;
    std::unique_ptr<ChatService> chatService_;
    std::unordered_map<TcpConnectionPtr, ConnectionState> connectionStates_;
};

#endif // CHATSERVER_H
