/*
 * =====================================================================================
 *
 *       Filename:  stream.h
 *
 *    Description:  IOStream
 *
 *        Version:  1.0
 *        Created:  2013年11月08日 23时54分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Y. T. Chung
 *   Organization:  
 *
 * =====================================================================================
 */

#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <deque>
#include <stdexcept>
#include <thread>
#include <mutex>
#include "ioloop.h"
#include "socket.h"

namespace httpserver {

    class IOStreamException : public std::runtime_error {
        public:
            explicit IOStreamException(const char *);
            explicit IOStreamException(const std::string&);
    };

    class IOStream {
        public:
            IOStream(const SocketClient& client, IOLoop& ioloop);
            ~IOStream();

            typedef std::function<void (const std::string& data, IOStream& stream)> DataHandler;

            void read_bytes(size_t len, const DataHandler& handler);
            void read_until(const char *until, const DataHandler& handler);

            void write_bytes(const void *buffer, size_t len);

            SocketClient client() const;
            void set_close_callback(const std::function<void (IOStream *)>&);
            void close();
        private:
            void __handler_poll(int fd, int type, void *arg, IOLoop& loop);
            size_t __read_to_buffer(SocketClient *client) throw (IOStreamException);
            bool __read_from_buffer();

            SocketClient _client;
            IOLoop& ioloop;
            bool _doing;
            bool _write_buf_freezing;
            int _read_num;
            const char *_read_until;
            DataHandler _read_handler;

            std::function<void (IOStream *)> _close_callback;

            int _send_buf_size;
            int _recv_buf_size;
            char *_send_buf;
            char *_recv_buf;
            std::deque<char> _rdbuf;
            std::deque<char> _wrbuf;

            std::mutex _rdmutex;
            std::mutex _wrmutex;

            bool _closed;
    };
}

