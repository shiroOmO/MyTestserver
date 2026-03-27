#include "TestServer.h"


int main() {
    EventLoop loop;
    InetAddress addr(9190, "127.0.0.1");
    TestServer server(&loop, addr, "mytestserver");

    server.start();
    loop.loop();
    return 0;
}



