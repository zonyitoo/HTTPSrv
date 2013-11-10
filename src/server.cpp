/*
 * =====================================================================================
 *
 *       Filename:  server.cpp
 *
 *    Description:  Server
 *
 *        Version:  1.0
 *        Created:  2013年11月10日 01时06分47秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <iterator>
#include "server.h"
#include "except.h"

namespace httpserver {
    HttpServer::HttpServer(int argc, char **argv, IOLoop& loop)
        : loop(loop) {
        
        // TODO: parse argument and fork
        
        loop.add_handler(server.Fd(), IOLoop::EV_READ, 
                [this] (int fd, int type, void *arg, IOLoop& loop) { this->__accept_handler(fd, type, arg, loop); }, &server);
    }

    HttpServer::~HttpServer() {
        loop.remove_handler(server.Fd());
        server.Close();
    }

    void HttpServer::register_handler(const std::string& urlpattern, HttpHandler *handler) {
        __regex_handler h;
        h.handler = handler;
        if (urlpattern[urlpattern.length() - 1] == '$') {
            h.regex = std::regex(urlpattern, std::regex_constants::extended);
            handlers[urlpattern] = h;
        }
        else {
            std::string ns = urlpattern + '$'; 
            h.regex = std::regex(ns, std::regex_constants::extended);
            handlers[ns] = h;
        }
    }

    void HttpServer::__accept_handler(int fd, int type, void *arg, IOLoop& loop) {
        TcpServer *server = static_cast<TcpServer *>(arg);
        while (true) {
            try {
                SocketClient client = server->Accept();
                std::clog << "Accepted connection from " << client.IPAddress() << std::endl;

                HttpConnection *conn = new HttpConnection(client, loop, [this] (const HttpRequest& req, HttpResponse& resp) {
                    std::smatch m;
                    std::vector<std::string> urlargs;
                    for (auto& h_iter : handlers) {
                        if (std::regex_match(req.uri, m, h_iter.second.regex, 
                                std::regex_constants::match_not_null)) {
                            copy(m.begin() + 1, m.end(), back_inserter(urlargs));
                        //if (h_iter.first == req.uri) {
                            HttpHandler *h = h_iter.second.handler;

                            h->prepare(req, resp, urlargs);

                            if (req.method == "GET")
                                h->get_handler(req, resp, urlargs);
                            else if (req.method == "POST")
                                h->post_handler(req, resp, urlargs);
                            else if (req.method == "PUT")
                                h->put_handler(req, resp, urlargs);
                            else if (req.method == "PATCH")
                                h->patch_handler(req, resp, urlargs);
                            else if (req.method == "DELETE")
                                h->delete_handler(req, resp, urlargs);
                            else if (req.method == "OPTIONS")
                                h->options_handler(req, resp, urlargs);
                            else if (req.method == "HEAD")
                                h->head_handler(req, resp, urlargs);
                            else if (req.method == "COPY")
                                h->copy_handler(req, resp, urlargs);
                            else if (req.method == "LINK")
                                h->link_handler(req, resp, urlargs);
                            else if (req.method == "UNLINK")
                                h->unlink_handler(req, resp, urlargs);
                            else
                                throw HttpError(405, "Method Not Allowed");

                            h->after(req, resp, urlargs);
                            return;
                        }
                    }

                    throw HttpError(404, "Not Found");
                });

                conn->set_close_callback([] (HttpConnection *conn) {
                    delete conn;
                });
            }
            catch (SocketError& except) {
                if (except.code() == EAGAIN || except.code() == EWOULDBLOCK) {
                    break;
                }
                else {
                    throw;
                }
            }
        }
    }
}
