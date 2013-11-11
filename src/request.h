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

#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include "headerstr.h"

namespace httpserver {
    struct Remote {
        std::string ip;
        uint16_t port;
    };

    struct HttpRequest {
        std::string version = "HTTP/1.1";
        std::string method = "GET";
        std::string uri = "/";
        HeaderMap headers;
        std::string raw_body;
        Remote remote;

        std::map<std::string, std::string> form;
        std::map<std::string, std::string> params;

        class ParseException : public std::invalid_argument {
            public:
                explicit ParseException(const std::string& what_arg);
                explicit ParseException(const char *what_arg);
        };

        void parse_headers(const std::string& raw_header) throw (ParseException);
        void parse_body(const std::string& raw_body) throw (ParseException);

    };
}

