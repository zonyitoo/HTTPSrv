/*
 * =====================================================================================
 *
 *       Filename:  stream.cpp
 *
 *    Description:  IOStream
 *
 *        Version:  1.0
 *        Created:  2013年11月09日 00时17分29秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "stream.h"
#include "ioloop.h"
#include <functional>
#include <unistd.h>
#include <climits>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <algorithm>

namespace httpserver {

    IOStreamException::IOStreamException(const char *what_arg)
        : std::runtime_error(what_arg) {}

    IOStreamException::IOStreamException(const std::string& what_arg)
        : std::runtime_error(what_arg) {}

    IOStream::IOStream(const SocketClient& client, IOLoop& loop)
        : _client(client), ioloop(loop), _doing(false), _write_buf_freezing(false),
          _read_num(-1), _read_until(nullptr), _closed(false) {

        ioloop.add_handler(_client.Fd(), IOLoop::EV_READ | IOLoop::EV_WRITE,
                [this] (int fd, int type, void *arg, IOLoop& loop) { __handler_poll(fd, type, arg, loop); }, &_client);

        unsigned int _sz = sizeof(_send_buf_size);
        if (getsockopt(_client.Fd(), SOL_SOCKET, SO_SNDBUF, &_send_buf_size, &_sz)) {
            perror("getsockopt");
            std::abort();
        }
        if (getsockopt(_client.Fd(), SOL_SOCKET, SO_RCVBUF, &_recv_buf_size, &_sz)) {
            perror("getsockopt");
            std::abort();
        }

        _send_buf = new char[_send_buf_size];
        _recv_buf = new char[_recv_buf_size];
    }

    IOStream::~IOStream() {
        close();
        delete [] _send_buf;
        delete [] _recv_buf;
    }

    SocketClient IOStream::client() const {
        return _client;
    }

    void IOStream::read_bytes(size_t len, const DataHandler& handler) {
        if (len == 0) {
            handler("", *this);
            return;
        }

        _read_num = len;
        _read_handler = handler;
        while (true) {
            //std::lock_guard<std::mutex> lck(this->_rdmutex);
            if (this->__read_from_buffer()) break;
            if (this->__read_to_buffer(&_client) == 0) break;
        }
    }

    void IOStream::read_until(const char *until, const DataHandler& handler) {
        if (!until) return;

        _read_until = until;
        _read_handler = handler;

        while (true) {
            //std::lock_guard<std::mutex> lck(this->_rdmutex);
            if (this->__read_from_buffer()) break;
            if (this->__read_to_buffer(&_client) == 0) break;
        }
    }

    void IOStream::write_bytes(const void *buffer, size_t len) {
        if (!buffer || len == 0) return;

        const char *_buf = static_cast<const char *>(buffer);
        //std::lock_guard<std::mutex> lck(this->_rdmutex);
        std::copy(_buf, _buf + len, back_inserter(_wrbuf));

        ioloop.update_handler(_client.Fd(), IOLoop::EV_READ | IOLoop::EV_WRITE);
    }

    void IOStream::__handler_poll(int fd, int type, void *arg, IOLoop& loop) {
        SocketClient *clientSocket = static_cast<SocketClient *>(arg);

        if (type & IOLoop::EV_READ) {
            //std::lock_guard<std::mutex> lck(this->_rdmutex);
            size_t result = 0;
            try {
                result = this->__read_to_buffer(clientSocket);
            }
            catch (const IOStreamException& except) {
                this->close();
                return;
            }

            if (result != 0) {
                this->__read_from_buffer();
            }
        }
        
        if (type & IOLoop::EV_WRITE) {
            //std::lock_guard<std::mutex> lck(this->_rdmutex);
            size_t n_avail = _wrbuf.size();
            if (n_avail > 0) {
                while (!_wrbuf.empty()) {

                    int total = (_wrbuf.size() < _send_buf_size) ? _wrbuf.size() : _send_buf_size;

                    auto _wrbuf_iter = _wrbuf.begin();
                    auto _end_iter = _wrbuf_iter + total;
                    int index = 0;
                    while (_wrbuf_iter != _end_iter) _send_buf[index ++] = *(_wrbuf_iter ++);

                    try {
                        total = clientSocket->Send(_send_buf, total);
                    }
                    catch (SocketError& except) {
                        if (except.code() == EAGAIN || except.code() == EWOULDBLOCK) {
                            break;
                        }
                        else {
                            std::cerr << "Socket send error: " << except.what() << std::endl;
                            goto ERROR_OCCUR;
                        }
                    }

                    while (total --) _wrbuf.pop_front();

                }
            }
        }
        return;
ERROR_OCCUR:
        this->close();
    }

    size_t IOStream::__read_to_buffer(SocketClient *client) throw (IOStreamException) {
        size_t readsize = 0;
        while (true) {
            int n;
            try {
                n = client->Recv(_recv_buf, _recv_buf_size);
                readsize += n;
                copy(_recv_buf, _recv_buf + n, back_inserter(_rdbuf));
            }
            catch (SocketError& except) {
                if (except.code() == EAGAIN || except.code() == EWOULDBLOCK) break;
                else {
                    std::cerr << "Socket recv error: " << except.what() << std::endl;
                    throw IOStreamException(except.what());
                }
            }
        }
        return readsize;
    }

    bool IOStream::__read_from_buffer() {
        if (_read_num != -1) {
            if (_rdbuf.size() >= _read_num && _rdbuf.size() != 0) {
                int _remain = _read_num;
                std::string data;
                data.reserve(_rdbuf.size());
                while (!_rdbuf.empty()) {
                    char x = _rdbuf.front();
                    _rdbuf.pop_front();
                    data.push_back(x);
                }
                _doing = false;
                _read_num = -1;
                if (_read_handler) _read_handler(data, *this); 
                return true;
            }
        }
        else if (_read_until) {
            size_t _until_str_len = strlen(_read_until);
            int char_step[256] = {0};
            for (size_t i = 0; i < 256; ++ i)
                char_step[i] = _until_str_len + 1;
            for (size_t i = 0; i < _until_str_len; ++ i)
                char_step[(size_t) _read_until[i]] = _until_str_len - i;

            auto itr = _rdbuf.begin();
            size_t subind = 0;
            while (itr != _rdbuf.end()) {
                auto tmp = itr;
                while (subind < _until_str_len) {
                    if (*itr == _read_until[subind]) {
                        itr ++;
                        subind ++;
                        continue;
                    }
                    else {
                        if (_rdbuf.end() - tmp < _until_str_len) {
                            // Could not find it.
                            goto SUNDAY_SEARCH_EXIT;
                        }

                        char firstRightChar = *(tmp + _until_str_len);
                        itr = tmp + char_step[(size_t) firstRightChar];
                        subind = 0;
                        break;
                    }
                }

                if (subind == _until_str_len) {
                    // Find it!!
                    std::string data;
                    size_t len = itr - _rdbuf.begin();
                    data.reserve(len);
                    while (len --) {
                        char x = _rdbuf.front();
                        _rdbuf.pop_front();
                        data.push_back(x);
                    }

                    _doing = false;
                    _read_until = nullptr;
                    if (_read_handler) _read_handler(data, *this);
                    return true;
                }
            }
SUNDAY_SEARCH_EXIT:
            ;
        }

        return false;
    }

    void IOStream::close() {
        if (_closed) return;

        _client.Close();
        ioloop.remove_handler(_client.Fd());

        _closed = true;
        if (_close_callback)
            _close_callback(this);
    }

    void IOStream::set_close_callback(const std::function<void (IOStream *)>& cb) {
        _close_callback = cb;
    }
}

