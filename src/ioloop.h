/*
 * =====================================================================================
 *
 *       Filename:  ioloop.h
 *
 *    Description:  IOLoop
 *
 *        Version:  1.0
 *        Created:  2013年11月08日 14时05分04秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Y. T. Chung
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

#include <sys/epoll.h>
#include <functional>
#include <array>
#include <stdexcept>
#include <string>
#include <memory>
#include <unordered_map>

namespace httpserver {

    class IOLoopException : public std::runtime_error {
        public:
            explicit IOLoopException(const std::string& what_arg);
            explicit IOLoopException(const char * what_arg);
    };

    class IOLoop {
        public:
            IOLoop();
            virtual ~IOLoop();

            enum EventType {
                EV_READ = 0x0001,
                EV_WRITE = 0x0002,
                EV_ERROR = 0x0004
            };

            enum EventFlag {
                EV_INITED = 0x0001,
                EV_ADDED = 0x0002,
                EV_ACTIVE = 0x0004,
                EV_DELETED = 0x0008
            };

            typedef std::function<void (int, int, IOLoop&)> EventCallback;

            virtual void add_handler(int fd, int event, const EventCallback& callback) throw (IOLoopException) = 0;
            virtual void update_handler(int fd, int event) throw (IOLoopException) = 0;
            virtual void remove_handler(int fd) throw (IOLoopException) = 0;

            virtual void start() = 0;
            virtual void stop() = 0;

        protected:
            void register_callback(int fd, EventCallback callback);
            void toggle_callback(int fd, int type);
            void remove_callback(int fd);

        private:
            std::unordered_map<int, EventCallback> handlers;
    };


    class EPollIOLoop : public IOLoop {
        public:
            EPollIOLoop();
            virtual ~EPollIOLoop();

            virtual void add_handler(int fd, int event, const EventCallback& callback) throw (IOLoopException);
            virtual void update_handler(int fd, int event) throw (IOLoopException);
            virtual void remove_handler(int fd) throw (IOLoopException);

            virtual void start() throw (IOLoopException);
            virtual void stop() throw (IOLoopException);
        private:
            int epoll_fd;

            static const size_t EPOLL_MAX_EVENT = 1024;
            std::array<struct epoll_event, EPOLL_MAX_EVENT> events;                        

            int wake_pipe[2];
    };
}

