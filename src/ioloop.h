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
//#include <thread>
//#include <mutex>
//#include <queue>
//#include <vector>
//#include <atomic>
//#include <condition_variable>

namespace httpserver {

    class IOLoopException : public std::runtime_error {
        public:
            explicit IOLoopException(const std::string& what_arg);
            explicit IOLoopException(const char * what_arg);
    };

    class IOLoop {
        public:
            IOLoop(int argc, char **argv);
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

            typedef std::function<void (int, int, void *, IOLoop&)> EventCallback;

            virtual void add_handler(int fd, int event, const EventCallback& callback, void *arg = nullptr) throw (IOLoopException);
            virtual void update_handler(int fd, int event) throw (IOLoopException) = 0;
            virtual void remove_handler(int fd) throw (IOLoopException);

            virtual int start() throw (IOLoopException);
            virtual void stop() throw (IOLoopException);

        protected:
            void toggle_callback(int fd, int type);

            struct IOEvent {
                int fd;
                EventCallback callback;
                void *arg;
                //std::shared_ptr<std::mutex> mutex_ptr;

                IOEvent();
                IOEvent(int, EventCallback, void *);
            };

            /*  
            struct ActiveEvent {
                int fd;
                int type;
            };
            */

        private:
            std::unordered_map<int, IOEvent> handlers;
            //std::vector<std::thread> threads;
            //std::queue<ActiveEvent> event_queue;
            //std::mutex queue_mutex;
            //std::condition_variable queue_cond;
            //int threadnum;
            //std::atomic<bool> started;
            bool started;
    };


    class EPollIOLoop : public IOLoop {
        public:
            EPollIOLoop(int argc, char **arg);
            virtual ~EPollIOLoop();

            virtual void add_handler(int fd, int event, const EventCallback& callback, void *arg = nullptr) throw (IOLoopException);
            virtual void update_handler(int fd, int event) throw (IOLoopException);
            virtual void remove_handler(int fd) throw (IOLoopException);

            virtual int start() throw (IOLoopException);
            virtual void stop() throw (IOLoopException);
        private:
            int epoll_fd;

            static const size_t EPOLL_MAX_EVENT = 1024;
            std::array<struct epoll_event, EPOLL_MAX_EVENT> events;                        

            int wake_pipe[2];
    };
}

