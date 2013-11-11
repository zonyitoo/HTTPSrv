/*
 * =====================================================================================
 *
 *       Filename:  headerstr.h
 *
 *    Description:  Header String traits
 *
 *        Version:  1.0
 *        Created:  2013年11月11日 09时20分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#pragma once

#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <functional>

namespace httpserver {
    struct ci_compare : std::binary_function<unsigned char, unsigned char, bool> {
        bool operator() (const unsigned char& c1, const unsigned char& c2) const {
            return tolower(c1) < tolower(c2);
        }
    };

    inline bool ci_equal(const std::string& str1, const std::string& str2) {
        return std::lexicographical_compare(str1.begin(), str1.end(),
                str2.begin(), str2.end(),
                ci_compare());
    }

    struct ci_less : std::binary_function<std::string, std::string, bool> {
        bool operator() (const std::string& str1, const std::string& str2) const {
            return ci_equal(str1, str2);
        }
    };

    typedef std::map<std::string, std::string, ci_less> HeaderMap;
}
