#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpServer.h"
#include "UChatService.h"

#include <cstdio>
#include <fcntl.h>
#include <functional>
#include <string>
#include <unistd.h>


class UChatServer {
public:
    UChatServer(EventLoop *loop, const InetAddress &addr, const std::string &name):
        server_(loop, addr, name), loop_(loop) {

        server_.setConnectionCallback(std::bind(&UChatServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&UChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        server_.setThreadNum(1);
    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn) {
        if(conn->connected())
            LOG_INFO("Connection from %s is UP.", conn->peerAddress().toIpPort().c_str());
        else {
            LOG_INFO("Connection from %s is DOWN.", conn->peerAddress().toIpPort().c_str());
            conn->shutdown();
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
        HttpParser pars;
        HttpResponse resp;

        if(!pars.parseRequest(buf))
            UChatService::instance().handleError(conn, pars.getRequest(), resp);
        else
            UChatService::instance().getHttpHandler(pars.getRequest().getPath())(conn, pars.getRequest(), resp);

        Buffer sendBuffer;
        resp.appendToBuffer(&sendBuffer);
        conn->send(sendBuffer);
        if(!resp.getKeepAlive())
            conn->shutdown();
    }

private:
    EventLoop *loop_;
    TcpServer server_;
};


int main() {
    EventLoop loop;
    InetAddress addr(9190, "0.0.0.0");
    UChatServer server(&loop, addr, "UChatServer");

    server.start();
    loop.loop();

    return 0;
}



