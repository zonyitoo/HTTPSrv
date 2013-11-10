/*
 * =====================================================================================
 *
 *       Filename:  except.cpp
 *
 *    Description:  Exceptions
 *
 *        Version:  1.0
 *        Created:  2013年11月09日 23时04分47秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <sstream>
#include "except.h"

namespace httpserver {
    HttpError::HttpError(unsigned int status_code, const std::string& msg)
        : std::logic_error(msg), _status_code(status_code) {}

    HttpError::HttpError(unsigned int status_code, const char *msg)
        : std::logic_error(msg), _status_code(status_code) {}

    HttpError::~HttpError() {}

    unsigned int HttpError::status_code() const {
        return _status_code;
    }

    const std::string HttpError::msg() const {
        return this->what();
    }

    std::string HttpError::make_package() const {
        static char error_html_fmt[] = "<html><head>"
            "<title>%u Error</title>"
            "</head><body>"
            "<h1 style=\"text-align:right\">%u %s</h1>"
            "<hr/>"
            "<h5 style=\"text-aligh:right\">C++11 Server 0.1</h5>"
            "</body>"
            "</html>";

        static char error_html_buf[1024] = {0};
        std::stringstream stream;
        stream << "HTTP/1.1 " << _status_code << " " << this->msg() << "\r\n";
        int len = sprintf(error_html_buf, error_html_fmt, _status_code, _status_code, this->msg().c_str());
        stream << "Content-Length: " << len << "\r\n";
        stream << "Content-Type: text/html\r\n";
        stream << "\r\n";
        stream << error_html_buf;

        return stream.str();
    }
}
