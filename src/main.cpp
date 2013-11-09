#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <map>
#include <algorithm>
#include <vector>
#include <sstream>
#include <regex>
#include <iostream>
#include <fcntl.h>

#include "connection.h"
#include "ioloop.h"
#include "stream.h"
using namespace httpserver;

/*
std::map<std::string, std::string> parse_http_package(char *p, size_t len) {
    std::map<std::string, std::string> contents;
    std::string package(p, len);

    std::regex reqex("([A-Z]+) +([^ ]+) +([^ \r\n]+)\r\n", 
            std::regex_constants::extended);
    std::regex headex("([A-Za-z0-9-]+): *([^ \r\n]+)\r\n",
            std::regex_constants::extended);

    std::smatch req;
    if (!std::regex_search(package, req, reqex)) std::abort();
    contents["Method"] = req[1];
    contents["Uri"] = req[2];
    contents["Version"] = req[3];
    package = req.suffix().str();

    std::smatch head;
    while (std::regex_search(package, head, headex)) {
        contents[head[1]] = head[2];
        package = head.suffix().str();
        if (package[0] == '\r') break;
    }

    if (package.length())

    contents["Body"] = package;
    
    return std::move(contents);
}
*/

char *Getline(char *pkg) {
    static char *pnt = nullptr;
    if (pkg) pnt = pkg;
    if (!pnt) return nullptr;

    if (*pnt == '\n') pnt ++;
    char *p = pnt;
    while (*pnt != '\r') pnt ++;
    *pnt ++ = '\0';
    return p;
}

char *parse_headers(char *pkg, std::map<std::string, std::string>& contents) {
    if (!pkg) return pkg;

    char *pnt = strpbrk(pkg, "\r\n");
    *pnt = '\0';
    pnt += 2;
    pkg = strtok(pkg, " ");
    contents["Method"] = pkg;
    pkg = strtok(nullptr, " ");
    contents["Uri"] = pkg;
    pkg = strtok(nullptr, " ");
    contents["Version"] = pkg;

    pkg = pnt;

    while (*pkg != '\r') {
        pnt = strpbrk(pkg, "\r\n");
        *pnt ++ = '\0';
        pnt ++;
        char *key = strtok(pkg, ": ");
        char *val = strtok(nullptr, "\0");
        while (*val == ' ') val ++;
        contents[key] = val;

        pkg = pnt;
    }

    pkg += 2;

    return pkg;
}

std::map<std::string, std::string> parse_http_package(char *pkg, size_t len) {
    std::map<std::string, std::string> contents;

    char *body = parse_headers(pkg, contents);

    size_t remain = len - (body - pkg);
    if (body && remain > 0) {
        contents["Body"] = std::string(body, remain);
    }

    return std::move(contents);
}

std::string escape(const std::string& s) {
    std::string res;
    for (char c : s) {
        switch (c) {
            case '\n':
                res += "\\n";
                break;
            case '\r':
                res += "\\r";
                break;
            case '"':
                res += "\\\"";
                break;
            default:
                res += c;
        }
    }
    return std::move(res);
}

std::string jsonbuilder(const std::map<std::string, std::string>& obj) {
    std::stringstream stream;
    stream << "{";
    bool first = true;
    for (auto kv : obj) {
        if (!first) 
            stream << ",";
        else
            first = false;
        auto k = escape(kv.first);
        auto v = escape(kv.second);
        stream << "\"" << k << "\":";
        stream << "\"" << v << "\"";
    }
    stream << "}";
    return stream.str();
}

IOStream *s = nullptr;
void handler_getbody(const std::string&);
std::map<std::string, std::string> contents;

void header_handler(const std::string& data) {
    std::cout << "--------------HEADER-------------------" << std::endl;
    std::cout << data << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    parse_headers(const_cast<char *>(data.c_str()), contents);

    long len = strtol(contents["Content-Length"].c_str(), nullptr, 10);
    std::clog << __FILE__ << ":" << __LINE__;
    std::clog << " Content-Length: " << len << std::endl;
    s->read_bytes(len, ptr_fun(handler_getbody));
}

void handler_getbody(const std::string& data) {
    std::cout << "---------------BODYPART---------------------" << std::endl;
    std::cout << data << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    contents["Body"] = data;
    char fmt[] = "HTTP/1.1 200 OK\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: application-json\r\n"
        "\r\n"
        "%s";
    std::string jsobj = jsonbuilder(contents);
    contents.clear();

    char buf[102400] = {0};
    int n = sprintf(buf, fmt, jsobj.size(), jsobj.c_str());
    s->write_bytes(buf, n);
    s->read_until("\r\n\r\n", ptr_fun(header_handler));
}

int main(int argc, char **argv) {
    /*  
    std::regex reqex("\\([A-Z]+\\) +\\(.+\\) +\\(.+\\)\r\n", 
            std::regex_constants::extended);
    */
    /*
    std::regex reqex("([A-Z]+)");
    std::string msg("POST");
    std::smatch m;
    std::cout << std::regex_search(msg, m, reqex) << std::endl;
    for (auto& a : m) {
        std::cout << a << std::endl;
    }
    */

    const char fmt[] = "HTTP/1.1 200 OK\r\n"
        "Content-Length: %zu\r\n"
        "Server: selfimp\r\n"
        "Date: Sat, 11 Jan 2003 02:44:04 GMT\r\n"
        "Content-Type: application/json\r\n"
        "Cache-control: private\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "%s";
    char buf[102400] = {0};

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8000);

    int sfd;
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

    listen(sfd, 5);

    /*
    while (true) {
        auto client = server.Accept();
        printf("Accept client: %s\n", client->IPAddress());
        int len;
        while ((len = client->Recv(buf, sizeof(buf))) > 0) {
            buf[len] = '\0';

            printf("%s\n", buf);

            auto c = parse_http_package(buf, len);
            std::string body = jsonbuilder(c);
            sprintf(buf, fmt, body.length(), body.c_str());
            client->Send(buf, strlen(buf));
        }
    }
    */
    EPollIOLoop ioloop;

    ioloop.add_handler(sfd, IOLoop::EV_READ, [] (int fd, int type, IOLoop &loop) {
        int cfd;
        struct sockaddr_in remoteaddr;
        socklen_t sin_size = sizeof(struct sockaddr_in);
        if ((cfd = accept(fd, (struct sockaddr *)&remoteaddr, &sin_size)) < 0) {
            perror("accept");
            std::abort();
        }
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

        std::clog << "Accepted connection from " << inet_ntoa(remoteaddr.sin_addr) << std::endl;

        s = new IOStream(cfd, loop);

        s->read_until("\r\n\r\n", ptr_fun(header_handler));
        /*
        loop.add_handler(cfd, IOLoop::EV_READ, [] (int fd, int type, IOLoop &loop) {
            if (type & IOLoop::EV_READ) {

                char buf[10240] = {0};
                int n = 0;
                if ((n = recv(fd, buf, sizeof(buf), 0)) < 0) {
                    perror("recv");
                    goto SHUTDOWN;
                    return;
                }

                std::cout << buf << std::endl;
                
                loop.update_handler(fd, IOLoop::EV_WRITE);
            }
            else if (type & IOLoop::EV_WRITE) {

                char buf[] = "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 18\r\n"
                    "Content-Type: application-json\r\n"
                    "\r\n"
                    "{\"hello\": \"world\"}";
                if (send(fd, buf, sizeof(buf), 0) < 0) {
                    perror("send");
                    goto SHUTDOWN;
                }

                loop.update_handler(fd, IOLoop::EV_READ);
            }
            else {
            SHUTDOWN:
                loop.remove_handler(fd);
                shutdown(fd, SHUT_RDWR);
            }
        });
        */
    });

    ioloop.start();

    return 0;
}
