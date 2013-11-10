/*
 * =====================================================================================
 *
 *       Filename:  server.h
 *
 *    Description:  CPP server
 *
 *        Version:  1.0
 *        Created:  2013年11月07日 01时11分15秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Y. T. Chung
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

#include <regex>
#include <string>
#include <unordered_map>
#include "handler.h"
#include "socket.h"
#include "ioloop.h"
#include "connection.h"

namespace httpserver {
    class HttpServer {
        public:
            HttpServer(int argc, char **argv, IOLoop& loop);
            ~HttpServer();
            HttpServer(const HttpServer&) = delete;
            HttpServer& operator=(const HttpServer&) = delete;

            void register_handler(const std::string& urlpattern, HttpHandler *handler);

        private:
            void __accept_handler(int, int, void *, IOLoop&);

            struct __regex_handler {
                std::regex regex;
                HttpHandler *handler;
            };

            std::unordered_map<std::string, __regex_handler> handlers;
            TcpServer server;

            IOLoop& loop;
    };
}

