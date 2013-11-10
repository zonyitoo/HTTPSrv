/*
 * =====================================================================================
 *
 *       Filename:  response.h
 *
 *    Description:  Response
 *
 *        Version:  1.0
 *        Created:  2013年11月07日 02时09分46秒
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
#include "except.h"

namespace httpserver {
    struct HttpResponse {
        unsigned int status_code = 200;
        std::string response_msg = "OK";
        std::string version = "HTTP/1.1";
        std::map<std::string, std::string> headers;
        std::string body; 

        std::string make_package();
    };
}
