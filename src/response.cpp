/*
 * =====================================================================================
 *
 *       Filename:  response.cpp
 *
 *    Description:  Response
 *
 *        Version:  1.0
 *        Created:  2013年11月09日 17时38分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "response.h"
#include <sstream>
#include <iostream>

namespace httpserver {

    std::string HttpResponse::make_package() {
        std::stringstream stream;
        stream << version << " " << status_code << " " << response_msg << "\r\n";

        auto itr = headers.find("Content-Type");
        if (itr == headers.end())
            headers["Content-Type"] = "text/plain";

        for (auto& itr : headers) {
            stream << itr.first << ": " << itr.second << "\r\n";
        }

        if (body.size() != 0) {
            stream << "Content-Length: " << body.size() << "\r\n";
        }

        stream << "\r\n";
        stream << body;

        return std::move(stream.str());
    }

}
