/*
 * =====================================================================================
 *
 *       Filename:  socket.cpp
 *
 *    Description:  Sockets
 *
 *        Version:  1.0
 *        Created:  2013年11月09日 16时49分51秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <memory>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include "socket.h"
#include <unistd.h>

namespace httpserver {

SocketError::SocketError(int code, const std::string& msg)
    : std::runtime_error(msg), _code(code) {}

SocketError::SocketError(int code, const char *msg)
    : std::runtime_error(msg), _code(code) {}

int SocketError::code() const {
    return _code;
}

SocketClient::SocketClient(int cfd, const struct sockaddr_in& addr, bool nonblock)
    : cfd(cfd), addr(addr) {

    if (nonblock) {
        int opts;
        if ((opts = fcntl(cfd, F_GETFL)) < 0) {
            perror("fcntl");
            std::abort();
        }

        opts = (opts | O_NONBLOCK);
        if (fcntl(cfd, F_SETFL, opts) < 0) {
            perror("fcntl");
            std::abort();
        }
    }
}

SocketClient::~SocketClient() {}

SocketClient::SocketClient(const SocketClient& rhs) 
    : cfd(rhs.cfd), addr(rhs.addr) {}

SocketClient::SocketClient(SocketClient&& rhs) 
    : cfd(rhs.cfd), addr(rhs.addr) {}

int SocketClient::Send(const char *buf, int len) throw (SocketError) {
    int ret = send(cfd, buf, len, 0);

    if (ret < 0) {
        throw SocketError(errno, strerror(errno));
    }
    else if (ret == 0) {
        throw SocketError(EREMOTEIO, "remote client closed");
    }
    return ret;
}

int SocketClient::Recv(char *buf, int len) throw (SocketError) {
    int ret = recv(cfd, buf, len, 0);
    if (ret < 0) {
        throw SocketError(errno, strerror(errno));
    }
    else if (ret == 0) {
        throw SocketError(EREMOTEIO, "remote client closed");
    }
    return ret;
}

const char *SocketClient::IPAddress() const {
    return inet_ntoa(addr.sin_addr);
}

uint16_t SocketClient::Port() const {
    return ntohs(addr.sin_port);
}

void SocketClient::Close() {
    if (shutdown(cfd, SHUT_RDWR) < 0) {
        perror("shutdown");
    }
    
    if (close(cfd) < 0) {
        perror("close");
    }
}

int SocketClient::Fd() const {
    return cfd;
}

TcpServer::TcpServer(uint16_t port, unsigned int queuelen, bool nonblock)
    : is_nonblock(nonblock) {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        std::abort();
    }

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        std::abort();
    }

    if (is_nonblock) {
        if ((opt = fcntl(sfd, F_GETFL)) < 0) {
            perror("fcntl");
            std::abort();
        }

        opt = (opt | O_NONBLOCK);
        if (fcntl(sfd, F_SETFL, opt) < 0) {
            perror("fcntl");
            std::abort();
        }
    }

    listen(sfd, queuelen);
}

TcpServer::~TcpServer() {
    this->Close();
}

SocketClient TcpServer::Accept() throw (SocketError) {
    int cfd;
    struct sockaddr_in remoteaddr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    if ((cfd = accept(sfd, (struct sockaddr *)&remoteaddr, &sin_size)) < 0) {
        throw SocketError(errno, strerror(errno));
    }

    return SocketClient(cfd, remoteaddr, is_nonblock);
}


void TcpServer::Close() {
    if (shutdown(sfd, SHUT_RDWR) < 0) {
        perror("shutdown");
    }

    if (close(sfd) < 0) {
        perror("close");
    }
}

int TcpServer::Fd() const {
    return sfd;
}

}

