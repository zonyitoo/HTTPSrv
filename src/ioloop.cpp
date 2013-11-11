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
#include <getopt.h>

namespace httpserver {

    IOLoopException::IOLoopException(const std::string& what_arg)
        : std::runtime_error(what_arg) {}

    IOLoopException::IOLoopException(const char * what_arg)
        : std::runtime_error(what_arg) {}

    IOLoop::IOLoop(int argc, char **argv)
        : started(false) {
        signal(SIGPIPE, SIG_IGN);

        /*
        static struct option _options_[] = {
                {"jobs", optional_argument, nullptr, 'j'},
                {nullptr, 0, nullptr, '\0'}
            };

        int c;
        while ((c = getopt_long(argc, argv, "j:", _options_, nullptr)) != -1) {
            switch (c) {
                case 'j':
                    threadnum = strtoul(optarg, nullptr, 10);
                    if (threadnum < 1) {
                        std::cerr << "Jobs should greater than or equal to 1" << std::endl;
                        std::abort();
                    }
                    break;
            }
        }
        */
    }

    IOLoop::~IOLoop() {}

    void IOLoop::add_handler(int fd, int event, const EventCallback& callback, void *arg) throw (IOLoopException) {
        handlers[fd] = IOEvent(fd, callback, arg);
    }

    void IOLoop::toggle_callback(int fd, int type) {
        /*
        ActiveEvent ev;
        ev.fd = fd;
        ev.type = type;
        std::unique_lock<std::mutex> qlock(queue_mutex);
        event_queue.push(ev);
        queue_cond.notify_one();
        */
        auto iter = handlers.find(fd);
        if (iter != handlers.end()) {
            iter->second.callback(fd, type, iter->second.arg, *this);
        }
    }

    void IOLoop::remove_handler(int fd) throw (IOLoopException) {
        auto iter = handlers.find(fd);
        if (iter != handlers.end())
            handlers.erase(iter);
    }

    int IOLoop::start() throw (IOLoopException) {
        this->started = true;
        /*
        for (int i = 0; i < threadnum; ++ i) {
            threads.push_back(std::thread([this]() {
                std::unique_lock<std::mutex> ulock(this->queue_mutex);
                while (this->started) {
                    while (this->event_queue.empty()) {
                        this->queue_cond.wait(ulock);
                    }

                    ActiveEvent ev = this->event_queue.front();
                    this->event_queue.pop();
                    auto iter = handlers.find(ev.fd);
                    if (iter != handlers.end()) {
                        std::lock_guard<std::mutex>(*iter->second.mutex_ptr);
                        iter->second.callback(ev.fd, ev.type, iter->second.arg, *this);
                    }
                }
            }));
        }
        */
        return 0;
    }

    void IOLoop::stop() throw (IOLoopException) {
        this->started = false;
        /*  
        this->queue_cond.notify_all();
        for (auto& t : threads)
            t.join();
        */
    }

    IOLoop::IOEvent::IOEvent() {}

    IOLoop::IOEvent::IOEvent(int fd, EventCallback cb, void *arg)
        : fd(fd), callback(cb), arg(arg) {}//, mutex_ptr(std::make_shared<std::mutex>()) {}

    EPollIOLoop::EPollIOLoop(int argc, char **argv) 
        : IOLoop(argc, argv) {
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
    
    void EPollIOLoop::add_handler(int fd, int type, const EventCallback& callback, void *arg) throw (IOLoopException) {
        struct epoll_event epev;
        memset(&epev, 0, sizeof(epev));

        if (type & EV_READ) epev.events |= EPOLLIN;
        if (type & EV_WRITE) epev.events |= EPOLLOUT;

        epev.events |= (EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLET);

        epev.data.fd = fd;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epev)) {
            perror("epoll_ctl");
            throw IOLoopException("EPoll add error");
        }

        IOLoop::add_handler(fd, type, callback, arg);
    }

    void EPollIOLoop::update_handler(int fd, int type) throw (IOLoopException) {
        struct epoll_event epev;
        memset(&epev, 0, sizeof(epev));

        if (type & EV_READ) epev.events |= EPOLLIN;
        if (type & EV_WRITE) epev.events |= EPOLLOUT;

        epev.events |= (EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLET);

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

        IOLoop::remove_handler(fd);
    }

    int EPollIOLoop::start() throw (IOLoopException) {
        IOLoop::start();
        int n;
        while ((n = epoll_wait(epoll_fd, events.data(), events.max_size(), -1)) >= 0) {
            for (int i = 0; i < n; ++ i) {
                int type = 0;

                if (events[i].data.fd == wake_pipe[0]) goto FINISHED;
                
                if (events[i].events & EPOLLIN) type |= EV_READ;
                
                if (events[i].events & EPOLLOUT) type |= EV_WRITE;

                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) type |= EV_ERROR;

                try {
                    toggle_callback(events[i].data.fd, type);
                }
                catch (std::exception& except) {
                    std::cerr << __FILE__ << ":" << __LINE__ << " " << except.what() << std::endl;
                    goto FAILED;
                }
            }
        }

FINISHED:
        return EXIT_SUCCESS;
FAILED:
        return EXIT_FAILURE;
    }

    void EPollIOLoop::stop() throw (IOLoopException) {
        char buf[] = "x";
        write(wake_pipe[1], buf, sizeof(buf));
    }

}
