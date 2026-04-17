#include "ChatServer.h"
#include "Protocol.h"
#include "Logger.h"
#include "json.hpp"
#include "Buffer.h"
#include "WebSocketHandler.h"

#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>


using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &addr, const std::string &name):
    server_(loop, addr, name), loop_(loop) {

    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server_.setThreadNum(3);
    chatService_.reset(new ChatService());

    loop_->runEvery(60 * 60 * 3, std::bind(&ChatServer::onTimer, this));
}

ChatServer::~ChatServer() {
    connectionStates_.clear();
}

void ChatServer::start() {
    server_.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn) {
    if(conn->connected()) {
        LOG_INFO("Connection from %s is UP", conn->peerAddress().toIpPort().c_str());
        // Initialize connection state
        ConnectionState state;
        state.isWebSocket = false;
        state.handshakeComplete = false;
        state.username = "";
        connectionStates_[conn] = state;
    }
    else {
        LOG_INFO("Connection from %s is DOWN", conn->peerAddress().toIpPort().c_str());
        auto it = connectionStates_.find(conn);
        if(it != connectionStates_.end() && !it->second.username.empty()) {
            chatService_->onDisconnect(it->second.username);
        }
        connectionStates_.erase(conn);
        conn->shutdown();
    }
}

bool ChatServer::handleHttpRequest(const TcpConnectionPtr &conn, const std::string &request) {
    // If this is a WebSocket upgrade request, don't handle it as static file
    if(WebSocketHandler::isWebSocketUpgrade(request)) {
        return false;  // Let it go to WebSocket handshake processing
    }

    // Parse request method and path
    if(request.substr(0, 3) == "GET") {
        size_t pathStart = 4;
        size_t pathEnd = request.find(" ", pathStart);
        if(pathEnd == std::string::npos) {
            return false;
        }
        
        std::string path = request.substr(pathStart, pathEnd - pathStart);

        // Default to chat.html
        std::string filePath;
        if(path == "/" || path == "/chat.html")
            filePath = "/home/admin/wcztmdd/MyTestserver/web/chat.html";
        else if(path == "/bg.png")
            filePath = "/home/admin/wcztmdd/MyTestserver/web/bg.png";
        else if(path == "/index.html")
            filePath = "/home/admin/wcztmdd/MyTestserver/web/index.html";
        else {
            // 404
            conn->send("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found");
            conn->shutdown();
            return true;
        }

        // Read file
        std::ifstream file(filePath, std::ios::binary);
        if(!file.is_open()) {
            conn->send("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found");
            conn->shutdown();
            return true;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Determine content type
        std::string contentType;
        if(filePath.find(".html") != std::string::npos)
            contentType = "text/html; charset=utf-8";
        else if(filePath.find(".png") != std::string::npos)
            contentType = "image/png";
        else
            contentType = "text/plain";

        // Build response
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: " << contentType << "\r\n"
                 << "Content-Length: " << content.size() << "\r\n"
                 << "Connection: close\r\n"
                 << "\r\n";

        conn->send(response.str());
        conn->send(content);
        conn->shutdown();
        return true;
    }

    // Not a GET request or not handled
    return false;
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
    auto &state = connectionStates_[conn];

    if(!state.handshakeComplete) {
        // Handshake not complete - need full request in buffer
        std::string data = buffer->retrieveAllAsString();
        // First check if it's a normal HTTP GET request for static files
        if(handleHttpRequest(conn, data)) {
            return;
        }

        // Check if this is a WebSocket upgrade request
        if(WebSocketHandler::isWebSocketUpgrade(data)) {
            state.isWebSocket = true;
            std::string response = WebSocketHandler::generateHandshakeResponse(data);
            conn->send(response);
            state.handshakeComplete = true;
            LOG_INFO("WebSocket handshake complete for %s", conn->peerAddress().toIpPort().c_str());
        }
        else {
            // Unsupported request
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\nBad Request");
            conn->shutdown();
        }
        return;
    }

    if(state.isWebSocket && state.handshakeComplete) {
        // Parse as many complete frames as available in the buffer
        while (buffer->readableBytes() > 0) {
            const char *dataPtr = buffer->peek();
            size_t readableLen = buffer->readableBytes();
            std::string payload;
            size_t frameLen;
            unsigned char opcode;

            if (WebSocketHandler::parseFrame(dataPtr, readableLen, payload, frameLen, opcode)) {
                // Got a complete frame - consume it
                if (opcode == 0x1) { // Text frame
                    if (!payload.empty()) {
                        handleWebSocketMessage(conn, payload);
                    }
                } else if (opcode == 0x8) { // Close frame - initiate close
                    LOG_INFO("Received WebSocket close frame from %s", conn->peerAddress().toIpPort().c_str());
                    conn->shutdown();
                    break;
                }
                buffer->retrieve(frameLen);
            } else {
                // Incomplete frame, leave remaining data in buffer for next read
                break;
            }
        }
    }
}

void ChatServer::onTimer() {
    chatService_->keepMySQLAlive();
}

void ChatServer::handleWebSocketMessage(const TcpConnectionPtr &conn, const std::string &payload) {
    auto &state = connectionStates_[conn];

    try {
        json j = json::parse(payload);
        std::string msgType = j[FIELD_MSG_TYPE];

        if(msgType == MSG_TYPE_REGISTER) {
            std::string username = j.value(FIELD_USERNAME, "");
            std::string password = j.value(FIELD_PASSWORD, "");
            chatService_->handleRegister(username, password, conn, state.username);
        }
        else if(msgType == MSG_TYPE_LOGIN) {
            std::string username = j.value(FIELD_USERNAME, "");
            std::string password = j.value(FIELD_PASSWORD, "");
            state.username = username;
            chatService_->handleLogin(username, password, conn);
            // Get message history after successful login
            chatService_->handleGetHistory(conn, 50);
        }
        else if(msgType == MSG_TYPE_LOGOUT) {
            chatService_->handleLogout(state.username, conn);
            state.username = "";
        }
        else if(msgType == MSG_TYPE_CHAT) {
            std::string content = j.value(FIELD_CONTENT, "");
            chatService_->handleChat(state.username, content);
        }
        else if(msgType == MSG_TYPE_HEARTBEAT) {
            chatService_->handleHeartbeat(state.username);
        }
        else if(msgType == MSG_TYPE_GET_HISTORY) {
            int limit = j.value(FIELD_LIMIT, 50);
            chatService_->handleGetHistory(conn, limit);
        }
        else {
            LOG_INFO("Unknown message type: %s", msgType.c_str());
        }
    }
    catch(const json::parse_error &e) {
        LOG_ERROR("JSON parse error: %s", e.what());
        json error;
        error[FIELD_MSG_TYPE] = MSG_TYPE_ERROR;
        error[FIELD_ERROR] = "Invalid JSON";
        std::string frame = WebSocketHandler::encodeFrame(error.dump());
        conn->send(frame);
    }
}

std::string ChatServer::getCurrentUsername(const TcpConnectionPtr &conn) {
    auto it = connectionStates_.find(conn);
    if(it != connectionStates_.end()) {
        return it->second.username;
    }
    return "";
}
