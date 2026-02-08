#pragma once

#include "Timestamp.h"
#include "noncopyable.h"

#include <unordered_map>
#include <vector>


class Channel;
class EventLoop;

class Poller: noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;
protected:
    typedef std::unordered_map<int, Channel*> ChannelMap;

public:
    Poller(EventLoop *loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;

    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    ChannelMap channels_;
private:
    EventLoop *ownerloop_;
};




