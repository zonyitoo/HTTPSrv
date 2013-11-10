/*
 * =====================================================================================
 *
 *       Filename:  handler.h
 *
 *    Description:  _handlers
 *
 *        Version:  1.0
 *        Created:  2013年11月07日 01时14分11秒
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
#include <vector>

#include "request.h"
#include "response.h"
#include "except.h"

namespace httpserver {
    class HttpHandler {
        public:
            HttpHandler();
            virtual ~HttpHandler();

            virtual void prepare(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);

            virtual void head_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void get_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void post_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void delete_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void put_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void options_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void patch_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void copy_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void link_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
            virtual void unlink_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);

            virtual void after(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs);
    };

}

