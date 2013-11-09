/*
 * =====================================================================================
 *
 *       Filename:  connection.h
 *
 *    Description:  HttpConnection
 *
 *        Version:  1.0
 *        Created:  2013年11月07日 16时42分13秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Y. T. Chung
 *   Organization:  
 *
 * =====================================================================================
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>

namespace httpserver {
    class TcpConnection {
        static int create(short port = 80, size_t queuelen = 5);

        static int accept(int fd);
    };
}

