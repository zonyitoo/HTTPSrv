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

    IOStream::IOStream(int fd, IOLoop& loop)
        : fd(fd), ioloop(loop), _doing(false), _write_buf_freezing(false),
          _read_num(-1), _read_until(nullptr) {
        ioloop.add_handler(fd, IOLoop::EV_READ | IOLoop::EV_WRITE, 
                [this] (int fd, int type, IOLoop& loop) { __handler_poll(fd, type, loop); });

        unsigned int _sz = sizeof(_send_buf_size);
        if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &_send_buf_size, &_sz)) {
            perror("getsockopt");
            std::abort();
        }
        if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &_recv_buf_size, &_sz)) {
            perror("getsockopt");
            std::abort();
        }

        _send_buf = new char[_send_buf_size];
        _recv_buf = new char[_recv_buf_size];
    }

    IOStream::~IOStream() {
        this->close();
        delete [] _send_buf;
        delete [] _recv_buf;
    }

    void IOStream::read_bytes(size_t len, const DataHandler& handler) {
        if (len == 0) {
            handler("");
            return;
        }

        _read_num = len;
        _read_handler = handler;
        while (true) {
            std::lock_guard<std::mutex> lck(this->_rdmutex);
            if (this->__read_from_buffer()) break;
            if (this->__read_to_buffer() == 0) break;
        }
    }

    void IOStream::read_until(const char *until, const DataHandler& handler) {
        if (!until) return;

        _read_until = until;
        _read_handler = handler;

        while (true) {
            std::lock_guard<std::mutex> lck(this->_rdmutex);
            if (this->__read_from_buffer()) break;
            if (this->__read_to_buffer() == 0) break;
        }
    }

    void IOStream::write_bytes(const void *buffer, size_t len) {
        if (!buffer || len == 0) return;

        const char *_buf = static_cast<const char *>(buffer);
        _write_buf_freezing = true;
        std::copy(_buf, _buf + len, back_inserter(_wrbuf));
        _write_buf_freezing = false;
    }

    void IOStream::__handler_poll(int fd, int type, IOLoop& loop) {
        std::clog << __FILE__ << ":" << __LINE__ << " ";
        std::clog << "fd=" << fd << std::hex << " type=" << type << std::endl;
        if (type & IOLoop::EV_READ) {
            std::lock_guard<std::mutex> lck(this->_rdmutex);
            size_t result = 0;
            try {
                result = this->__read_to_buffer();
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
            std::lock_guard<std::mutex> lck(this->_wrmutex);
            size_t n_avail = _wrbuf.size();
            std::clog << __FILE__ << ":" << __LINE__ << " _wrbuf.size() = " << _wrbuf.size() << std::endl;
            if (n_avail > 0) {

                int cnt = 0;
                while (!_wrbuf.empty() && cnt < _send_buf_size) {
                    char x = _wrbuf.front();
                    _wrbuf.pop_front();
                    _send_buf[cnt ++] = x;
                }
                if (send(fd, _send_buf, cnt, 0) < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        perror("send");
                        return;
                    }
                    else {
                        goto ERROR_OCCUR;
                    }
                }
            }
        }
        else {
ERROR_OCCUR:
            this->close();
        }
    }

    size_t IOStream::__read_to_buffer() throw (IOStreamException) {
        int n;
        if ((n = recv(fd, _recv_buf, _recv_buf_size, 0)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                perror("recv");
                return 0;
            }
            else {
                throw IOStreamException(strerror(errno));
            }
        }

        copy(_recv_buf, _recv_buf + n, back_inserter(_rdbuf));

        return n;
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
                if (_read_handler) _read_handler(data); 
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
                            std::clog << __FILE__ << ":" << __LINE__ << " Could not find it"<< std::endl;
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
                    if (_read_handler) _read_handler(data);
                    return true;
                }
            }
SUNDAY_SEARCH_EXIT:
            ;
        }

        return false;
    }

    void IOStream::close() {
        shutdown(fd, SHUT_RDWR);
        ioloop.remove_handler(fd);
    }
}

