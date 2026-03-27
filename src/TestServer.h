#include "TcpServer.h"

#include <string>


class TestServer {
public:
    TestServer(EventLoop *loop, const InetAddress &addr, const std::string &name);
    ~TestServer();
    void start();
private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);

private:
    EventLoop *loop_;
    TcpServer server_;
};

