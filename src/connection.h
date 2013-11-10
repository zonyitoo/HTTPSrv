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

#pragma once

#include <functional>
#include "socket.h"
#include "request.h"
#include "response.h"
#include "stream.h"

namespace httpserver {
    class HttpConnection {
        public:
            typedef std::function<void (const HttpRequest&, HttpResponse&)> HttpConnectionHandler;

            HttpConnection(const SocketClient& client, IOLoop& loop, const HttpConnectionHandler& handler);
            ~HttpConnection();

            void set_close_callback(const std::function<void (HttpConnection *)>&);
            void close();

        private:
            void __stream_handler_get_header(const std::string&, IOStream&) noexcept;
            void __stream_handler_get_body(const std::string&, IOStream&) noexcept;

            IOStream _stream;
            HttpConnectionHandler handler;
            HttpRequest request;

            std::function<void (HttpConnection *)> _close_callback;
            bool _closed;
    };
}
