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
#include "headerstr.h"

namespace httpserver {
    struct HttpResponse {
        unsigned int status_code = 200;
        std::string response_msg = "OK";
        std::string version = "HTTP/1.1";
        HeaderMap headers;
        std::string body; 
        bool close = false;

        std::string make_package();
    };
}
