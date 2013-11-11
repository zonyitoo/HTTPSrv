#include <iostream>

#include "connection.h"
#include "ioloop.h"
#include "stream.h"
#include "socket.h"
#include "request.h"
#include "server.h"
#include "handler.h"
using namespace httpserver;
using namespace std;

class HelloWorldHandler : public HttpHandler {
    public:
        void get_handler(const HttpRequest& req, HttpResponse& resp, const std::vector<std::string>& args) {
            resp.headers["Content-Type"] = "application/json";
            resp.headers["Server"] = "YTWS/1.0";

            resp.body = "{";
            for (auto& itr : req.params) {
                resp.body += "\"";
                resp.body += itr.first;
                resp.body += "\":\"";
                resp.body += itr.second;
                resp.body += "\",";
            }

            resp.body += "\"urlargs\": [ ";
            for (auto& itr : args) {
                resp.body += "\"";
                resp.body += itr;
                resp.body += "\",";
            }
            resp.body[resp.body.size() - 1] = ']';
            resp.body += "}";
        }

        void post_handler(const HttpRequest& req, HttpResponse& resp, const std::vector<std::string>& args) {
            resp.body = req.raw_body;
        }
};

int main(int argc, char **argv) {

    EPollIOLoop ioloop(argc, argv);

    HttpServer server(8000, 200, ioloop);
    server.register_handler("/", new HelloWorldHandler());
    server.register_handler("/([^/]*)/", new HelloWorldHandler());

    return ioloop.start();
}
