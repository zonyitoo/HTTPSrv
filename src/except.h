/*
 * =====================================================================================
 *
 *       Filename:  except.h
 *
 *    Description:  Exceptions
 *
 *        Version:  1.0
 *        Created:  2013年11月07日 01时25分28秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

#include <stdexcept>
#include <string>

namespace httpserver {
    class HttpError : protected std::logic_error {
        public:
            explicit HttpError(unsigned int status_code = 500, const std::string& msg = "Internal Server Error");
            explicit HttpError(unsigned int status_code = 500, const char *msg = "Internal Server Error");
            virtual ~HttpError();

            unsigned int status_code() const;
            const std::string msg() const;

            std::string make_package() const;
        private:
            int _status_code;
    };
}
