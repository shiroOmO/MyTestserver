#include "ChatServer.h"


int main() {
    EventLoop loop;
    InetAddress addr(9190, "0.0.0.0");
    ChatServer server(&loop, addr, "mytestserver");

    server.start();
    loop.loop();
    return 0;
}



