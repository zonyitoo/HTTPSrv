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

#include <functional>
#include <iostream>
#include "connection.h"
#include "except.h"

namespace httpserver {
    
    HttpConnection::HttpConnection(const SocketClient& client, IOLoop& loop, const HttpConnectionHandler& handler) 
        : _stream(client, loop), handler(handler), _closed(false) {

        _stream.set_close_callback([this] (IOStream *) { this->close(); });

        _stream.read_until("\r\n\r\n", [this] (const std::string& data, IOStream& stream) {
                __stream_handler_get_header(data, stream); });
    }

    HttpConnection::~HttpConnection() {
        this->close();
    }

    void HttpConnection::__stream_handler_get_header(const std::string& data, IOStream& stream) noexcept {
        long len;
        request.remote.ip = stream.client().IPAddress();
        request.remote.port = stream.client().Port();
        try {
            request.parse_headers(data); 
        }
        catch (HttpRequest::ParseException& except) {

            std::clog << __FILE__ << ":" << __LINE__ << " ";
            std::clog << except.what() << std::endl;

            goto restart;
        }

        len = strtol(request.headers["Content-Length"].c_str(), nullptr, 10);
        stream.read_bytes(len, [this] (const std::string& data, IOStream& stream) {
                __stream_handler_get_body(data, stream); });
        return;
restart:
        stream.read_until("\r\n\r\n", [this] (const std::string& data, IOStream& stream) {
                __stream_handler_get_header(data, stream); });
    }

    void HttpConnection::__stream_handler_get_body(const std::string& data, IOStream& stream) noexcept {
        HttpResponse response;

        try {
            this->request.parse_body(data);

            this->handler(request, response);

            std::clog << "[" << request.remote.ip << ":" << request.remote.port << "] ";
            std::clog << request.method << " " << request.uri << " ";
            std::clog << response.status_code << " " << response.response_msg << std::endl;
            std::string resp = response.make_package();
            stream.write_bytes(resp.c_str(), resp.length());
        }
        catch (HttpError& error) {
            std::clog << "[" << request.remote.ip << ":" << request.remote.port << "] ";
            std::clog << request.method << " " << request.uri << " ";
            std::string resp = error.make_package();
            std::clog << error.status_code() << " " << error.msg() << std::endl;
            stream.write_bytes(resp.c_str(), resp.length());
        }
        catch (HttpRequest::ParseException& except) {

            std::clog << __FILE__ << ":" << __LINE__ << " ";
            std::clog << except.what() << std::endl;

        }

        stream.read_until("\r\n\r\n", [this] (const std::string& data, IOStream& stream) {
                __stream_handler_get_header(data, stream); });
    }

    void HttpConnection::close() {
        if (_closed) return;
        _stream.close();
        _closed = true;

        if (_close_callback)
            _close_callback(this);
    }

    void HttpConnection::set_close_callback(const std::function<void (HttpConnection *)>& cb) {
        _close_callback = cb;
    }

}
