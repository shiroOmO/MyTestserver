#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <sys/epoll.h>
#include <vector>


class Channel;

class EPollPoller: public Poller {
private:
    typedef std::vector<epoll_event> EventList;

public:
    EPollPoller(EventLoop *loop); 
    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    void update(int operation, Channel *channel);

private:
    static const int kInitEventListSize = 16;
    int epollfd_;
    EventList events_;
};


