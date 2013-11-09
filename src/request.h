/*
 * =====================================================================================
 *
 *       Filename:  request.h
 *
 *    Description:  Request
 *
 *        Version:  1.0
 *        Created:  2013年11月07日 01时22分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Y. T. Chung
 *   Organization:  
 *
 * =====================================================================================
 */

#include <string>
#include <map>
#include <vector>


namespace httpserver {
    class HTTPRequest {
        public:
            enum HTTPMethod {
                UNKNOWN_HTTP_METHOD = 0,
                GET = 0x01,
                POST = 0x02,
                DELETE = 0x04,
                PUT = 0x08,
                OPTIONS = 0x10,
                HEAD = 0x20,
                TRACE = 0x40,
                CONNECT = 0x80
            };

            enum HTTPVersion {
                UNKNOWN_HTTP_VERSION,
                HTTP1_0,
                HTTP1_1
            };

            enum HTTPProtocol {
                UNKNOWN_HTTP_PROTOCOL,
                HTTP,
                HTTPS
            };

            HTTPMethod(HTTPMethod, 
                    const std::string& uri = "",
                    HTTPVersion version = HTTP1_0, 
                    HTTPProtocol protocol = UNKNOWN_HTTP_PROTOCOL,
                    const std::map<std::string, std::string>& headers,
                    const std::string& body,
                    const std::string& remote_ip,
                    const std::string& host);

            HTTPMethod method() const;
            const std::string& uri() const;
            const std::string& path() const;
            HTTPVersion version() const;
            HTTPProtocol protocol() const;
            const std::map<std::string, std::string>& headers() const;
            const std::string& remote_ip() const;

        private:
            HTTPMethod _method;
            std::string _uri;
            std::string _path;
            HTTPVersion _version;
            HTTPProtocol _protocol;
            std::map<std::string, std::string> _headers;
            std::string _body;
            std::string _remote_ip;
            std::map<std::string, std::string> _arguments;
            std::map<std::string, std::string> _query_arguments;
            std::map<std::string, std::string> _body_arguments;
    };
}

