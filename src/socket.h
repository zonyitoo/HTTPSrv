/*
 * =====================================================================================
 *
 *       Filename:  socket.h
 *
 *    Description:  Sockets
 *
 *        Version:  1.0
 *        Created:  2013年11月09日 16时49分21秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <stdexcept>

namespace httpserver {
    class SocketError : public std::runtime_error {
        public:
            explicit SocketError(int fd, int code, const std::string& msg);
            explicit SocketError(int fd, int code, const char *msg);
            int code() const;
            int fd() const;
        private:
            int _code;
            int _fd;
    };

    class SocketClient {
        public:
            SocketClient(int cfd, const struct sockaddr_in& addr, bool nonblock = true);

            ~SocketClient();

            SocketClient(const SocketClient& rhs);
            SocketClient(SocketClient&& rhs);

            int send(const char *buf, int len) throw (SocketError);

            int recv(char *buf, int len) throw (SocketError);

            const char *ip_address() const;
            int fd() const;
            uint16_t port() const;

            void close();
            void shutdown(int how = SHUT_RD);
        private:
            int cfd;
            struct sockaddr_in addr;
    };

    class TcpServer {
        public:
            TcpServer(uint16_t port = 8000, unsigned int queuelen = 5, bool nonblock = true);

            ~TcpServer();

            TcpServer(const TcpServer&) = delete;
            TcpServer& operator=(const TcpServer&) = delete;
            int fd() const;

            SocketClient accept() throw (SocketError);

            void close();
            void shutdown(int how = SHUT_RD);
        private:
            int sfd;
            struct sockaddr_in addr;
            bool is_nonblock;
    };
}

