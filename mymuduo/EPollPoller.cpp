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
        LOG_FATAL("epoll fd create fail, exit, errno=%d.", errno);
}

EPollPoller::~EPollPoller() {
    close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    LOG_DEBUG("epoll fd=%d will epoll_wait(), total fd count that be listend is %lu.", epollfd_, channels_.size());

    int numEvents = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0) {
        LOG_DEBUG("epoll fd=%d's epoll_wait() return %d events.", epollfd_, numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size())
            events_.resize(events_.size() * 2);
    }
    else if(numEvents == 0)
        LOG_DEBUG("epoll fd=%d's epoll_wait() timeout.", epollfd_);
    else {
        if(saveErrno != EINTR) {
            errno = saveErrno;
            LOG_ERROR("epoll fd=%d's epoll_wait() fail, do not get event, errno=%d.", epollfd_, errno);
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel) {
    const int index = channel->index();
    LOG_DEBUG("socket fd=%d's channel will update event, events=%d, index=%d.", channel->fd(), channel->events(), index);
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

    LOG_DEBUG("socket fd=%d's channel will remove.", fd);

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
            LOG_ERROR("socket fd=%d's EPOLL_CTL_DEL fail, errno=%d.", fd, errno);
        else
            LOG_FATAL("socket fd=%d's EPOLL_CTL_ADD/MOD fail, errno=%d.", fd, errno);
    }

}



