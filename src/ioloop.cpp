/*
 * =====================================================================================
 *
 *       Filename:  ioloop.cpp
 *
 *    Description:  IOLoop Implementation
 *
 *        Version:  1.0
 *        Created:  2013年11月08日 14时34分42秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "ioloop.h"
#include <sys/epoll.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <iostream>

namespace httpserver {

    IOLoopException::IOLoopException(const std::string& what_arg)
        : std::runtime_error(what_arg) {}

    IOLoopException::IOLoopException(const char * what_arg)
        : std::runtime_error(what_arg) {}

    IOLoop::IOLoop() {
        signal(SIGPIPE, SIG_IGN);
    }

    IOLoop::~IOLoop() {}

    void IOLoop::register_callback(int fd, EventCallback callback) {
        handlers[fd] = callback;
    }

    void IOLoop::toggle_callback(int fd, int type) {
        auto iter = handlers.find(fd);
        if (iter != handlers.end())
            iter->second(fd, type, *this);
    }

    void IOLoop::remove_callback(int fd) {
        auto iter = handlers.find(fd);
        if (iter != handlers.end())
            handlers.erase(iter);
    }

    EPollIOLoop::EPollIOLoop() {
        if ((epoll_fd = epoll_create(EPOLL_MAX_EVENT)) < 0) {
            perror("epoll_create");
            exit(EXIT_FAILURE);
        }

        if (pipe(wake_pipe) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = wake_pipe[0];
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, wake_pipe[0], &ev)) {
            perror("epoll_ctl");
            exit(EXIT_FAILURE);
        }
    }

    EPollIOLoop::~EPollIOLoop() {
        close(epoll_fd);
        close(wake_pipe[0]);
        close(wake_pipe[1]);
    }
    
    void EPollIOLoop::add_handler(int fd, int type, const EventCallback& callback) throw (IOLoopException) {
        struct epoll_event epev;
        memset(&epev, 0, sizeof(epev));

        if (type & EV_READ) epev.events |= EPOLLIN | EPOLLET;
        if (type & EV_WRITE) epev.events |= EPOLLOUT;

        epev.events |= (EPOLLERR | EPOLLHUP | EPOLLRDHUP);

        epev.data.fd = fd;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epev)) {
            perror("epoll_ctl");
            throw IOLoopException("EPoll add error");
        }

        register_callback(fd, callback);
    }

    void EPollIOLoop::update_handler(int fd, int type) throw (IOLoopException) {
        struct epoll_event epev;
        memset(&epev, 0, sizeof(epev));

        if (type & EV_READ) epev.events |= EPOLLIN | EPOLLET;
        if (type & EV_WRITE) epev.events |= EPOLLOUT;

        epev.events |= (EPOLLERR | EPOLLHUP | EPOLLRDHUP);

        epev.data.fd = fd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &epev)) {
            perror("epoll_ctl");
            throw IOLoopException("EPoll add error");
        }
    }

    void EPollIOLoop::remove_handler(int fd) throw (IOLoopException) {
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr)) {
            perror("epoll_ctl");
            throw IOLoopException("EPoll del error");
        }

        remove_callback(fd);
    }

    void EPollIOLoop::start() throw (IOLoopException) {
        int n;
        while ((n = epoll_wait(epoll_fd, events.data(), events.max_size(), -1)) >= 0) {
            for (int i = 0; i < n; ++ i) {
                int type = 0;

                if (events[i].data.fd == wake_pipe[0]) goto FINISHED;
                
                if (events[i].events & EPOLLIN) type |= EV_READ;
                
                if (events[i].events & EPOLLOUT) type |= EV_WRITE;

                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) type |= EV_ERROR;

                std::clog << __FILE__ << ":" << __LINE__ << " Got event ";
                std::clog << "READ: " << bool(type & EV_READ) << " ";
                std::clog << "WRITE: " << bool(type & EV_WRITE) << " ";
                std::clog << "ERROR: " << bool(type & EV_ERROR) << std::endl;

                toggle_callback(events[i].data.fd, type);
            }
        }

FINISHED:
        return;
    }

    void EPollIOLoop::stop() throw (IOLoopException) {
        char buf[] = "x";
        write(wake_pipe[1], buf, sizeof(buf));
    }

}
