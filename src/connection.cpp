/*
 * =====================================================================================
 *
 *       Filename:  connection.cpp
 *
 *    Description:  HttpConnection
 *
 *        Version:  1.0
 *        Created:  2013年11月07日 16时43分04秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Y. T. Chung
 *   Organization:  
 *
 * =====================================================================================
 */
#include "connection.h"
#include <memory>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>

namespace httpserver {

TcpClient::TcpClient(int cfd, const struct sockaddr_in& addr)
    : cfd(cfd), addr(addr) {

    int opts;
    if ((opts = fcntl(cfd, F_GETFL)) < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    opts = (opts | O_NONBLOCK);
    if (fcntl(cfd, F_SETFL, opts) < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

TcpClient::~TcpClient() {}

TcpClient::TcpClient(const TcpClient& rhs) 
    : cfd(rhs.cfd), addr(rhs.addr) {}

TcpClient::TcpClient(TcpClient&& rhs) 
    : cfd(rhs.cfd), addr(rhs.addr) {}

int TcpClient::Send(const char *buf, int len) {
    return send(cfd, buf, len, 0);
}

int TcpClient::Recv(char *buf, int len) {
    return recv(cfd, buf, len, 0);
}


char *TcpClient::IPAddress() const {
    return inet_ntoa(addr.sin_addr);
}

void TcpClient::Close() {
    shutdown(cfd, SHUT_RDWR);
}

int TcpClient::fd() const {
    return cfd;
}

TcpServer::TcpServer(short port, unsigned int queuelen) {
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

    listen(sfd, queuelen);
}

TcpServer::~TcpServer() {
    this->Close();
}

int TcpServer::Accept() {
    int cfd;
    struct sockaddr_in remoteaddr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    if ((cfd = accept(sfd, (struct sockaddr *)&remoteaddr, &sin_size)) < 0) {
        perror("accept");
        std::abort();
    }

    std::clog << "Accepted connection from " << inet_ntoa(remoteaddr.sin_addr) << std::endl;

    return cfd;
}


void TcpServer::Close() {
    shutdown(sfd, SHUT_RDWR);
}

int TcpServer::Fd() const {
    return sfd;
}

}
