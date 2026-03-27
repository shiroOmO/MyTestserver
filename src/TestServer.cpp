#include "TestServer.h"


TestServer::TestServer(EventLoop *loop, const InetAddress &addr, const std::string &name):
    server_(loop, addr, name), loop_(loop) {

    server_.setConnectionCallback(std::bind(&TestServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&TestServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    server_.setThreadNum(3);
}

TestServer::~TestServer() {}

void TestServer::start() {
    server_.start();
}

void TestServer::onConnection(const TcpConnectionPtr &conn) {
    if(conn->connected())
        LOG_INFO("Connection from %s is UP.", conn->peerAddress().toIpPort().c_str());
    else {
        LOG_INFO("Connection from %s is DOWN.", conn->peerAddress().toIpPort().c_str());
        conn->shutdown();
    }
}

void TestServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
    std::string buf = buffer->retrieveAllAsString();
    conn->send(buf);
}



