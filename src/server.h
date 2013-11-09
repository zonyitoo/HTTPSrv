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


namespace httpserver {
    class HTTPServer {
        public:
            HTTPServer(int argc, char **argv);
            ~HTTPServer();
            HTTPServer(const HTTPServer&) = delete;
            HTTPServer& operator=(const HTTPServer&) = delete;

            void registerHandler(const std::string& pattern, 
                    HTTPRequest::HTTPMethod methods,
                    const std::function<void (const HTTPRequest&, HTTPResponse& resp)>&);

            int run();
    };
}

