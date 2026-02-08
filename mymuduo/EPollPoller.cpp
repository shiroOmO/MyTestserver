#include "Channel.h"
#include "EPollPoller.h"
#include "Logger.h"
#include "Timestamp.h"

#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>


const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop):
    Poller(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize) {
    if(epollfd_ < 0)
        LOG_FATAL("%s:%s:%d => epoll fd create fail, exit, errno=%d.", __FILENAME__, __FUNCTION__, __LINE__, errno);
}

EPollPoller::~EPollPoller() {
    close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    LOG_DEBUG("%s:%s:%d => epoll fd=%d will epoll_wait(), total fd count that be listend is %lu.", __FILENAME__, __FUNCTION__, __LINE__, epollfd_, channels_.size());

    int numEvents = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0) {
        LOG_DEBUG("%s:%s:%d => epoll fd=%d's epoll_wait() return %d events.", __FILENAME__, __FUNCTION__, __LINE__, epollfd_, numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size())
            events_.resize(events_.size() * 2);
    }
    else if(numEvents == 0)
        LOG_DEBUG("%s:%s:%d => epoll fd=%d's epoll_wait() timeout.", __FILENAME__, __FUNCTION__, __LINE__, epollfd_);
    else {
        if(saveErrno != EINTR) {
            errno = saveErrno;
            LOG_ERROR("%s:%s:%d => epoll fd=%d's epoll_wait() fail, do not get event, errno=%d.", __FILENAME__, __FUNCTION__, __LINE__, epollfd_, errno);
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel) {
    const int index = channel->index();
    LOG_DEBUG("%s:%s:%d => socket fd=%d's channel will update event, events=%d, index=%d.", __FILENAME__, __FUNCTION__, __LINE__, channel->fd(), channel->events(), index);
    if(index == kNew || index == kDeleted) {
        if(index == kNew) {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        int fd = channel->fd();
        if(channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) {
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_DEBUG("%s:%s:%d => socket fd=%d's channel will remove.", __FILENAME__, __FUNCTION__, __LINE__, fd);

    int index = channel->index();
    if(index == kAdded)
        update(EPOLL_CTL_DEL, channel);
    channel->set_index(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for(int i = 0; i < numEvents; i++) {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::update(int operation, Channel *channel) {
    epoll_event event;
    memset(&event, 0, sizeof event);

    int fd = channel->fd();
    event.events = channel->events();
    event.data.fd = fd; 
    event.data.ptr = channel;

    if(epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if(operation == EPOLL_CTL_DEL)
            LOG_ERROR("%s:%s:%d => socket fd=%d's EPOLL_CTL_DEL fail, errno=%d.", __FILENAME__, __FUNCTION__, __LINE__, fd, errno);
        else
            LOG_FATAL("%s:%s:%d => socket fd=%d's EPOLL_CTL_ADD/MOD fail, errno=%d.", __FILENAME__, __FUNCTION__, __LINE__, fd, errno);
    }

}



