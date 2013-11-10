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
            explicit SocketError(int code, const std::string& msg);
            explicit SocketError(int code, const char *msg);
            int code() const;
        private:
            int _code;
    };

    class SocketClient {
        public:
            SocketClient(int cfd, const struct sockaddr_in& addr, bool nonblock = true);

            ~SocketClient();

            SocketClient(const SocketClient& rhs);
            SocketClient(SocketClient&& rhs);

            int Send(const char *buf, int len) throw (SocketError);

            int Recv(char *buf, int len) throw (SocketError);

            const char *IPAddress() const;
            int Fd() const;
            uint16_t Port() const;

            void Close();
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
            int Fd() const;

            SocketClient Accept() throw (SocketError);

            void Close();
        private:
            int sfd;
            struct sockaddr_in addr;
            bool is_nonblock;
    };
}

