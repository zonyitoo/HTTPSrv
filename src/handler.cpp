/*
 * =====================================================================================
 *
 *       Filename:  handler.cpp
 *
 *    Description:  Handler
 *
 *        Version:  1.0
 *        Created:  2013年11月10日 00时15分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Y. T. Chung
 *   Organization:  
 *
 * =====================================================================================
 */

#include "handler.h"

namespace httpserver {
    HttpHandler::HttpHandler() {}
    HttpHandler::~HttpHandler() {}

    void HttpHandler::prepare(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {}

    void HttpHandler::head_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");
    }

    void HttpHandler::get_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::post_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::delete_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::put_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::options_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::copy_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::link_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::unlink_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::patch_handler(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {
        throw HttpError(405, "Method Not Allowed");

    }

    void HttpHandler::after(const HttpRequest& request, HttpResponse& response, const std::vector<std::string>& urlmatchs) {}

}
