/*
 * =====================================================================================
 *
 *       Filename:  handler.h
 *
 *    Description:  Handlers
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

#include <string>

#include "request.h"
#include "response.h"
#include "except.h"

namespace httpserver {
    class HTTPHandler {
        public:
            HTTPHandler();
            HTTPHandler(const std::string& urlpattern);
            virtual ~HTTPHandler();

            virtual prepare(HTTPRequest& request, HTTPResponse& response);

            virtual headHandler(const HTTPRequest& request, HTTPResponse& response);
            virtual getHandler(const HTTPRequest& request, HTTPResponse& response);
            virtual postHandler(const HTTPRequest& request, HTTPResponse& response);
            virtual deleteHandler(const HTTPRequest& request, HTTPResponse& response);
            virtual putHandler(const HTTPRequest& request, HTTPResponse& response);
            virtual optionsHandler(const HTTPRequest& request, HTTPResponse& response);
    };

}

