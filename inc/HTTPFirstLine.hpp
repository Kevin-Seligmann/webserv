#pragma once

#include "HTTPElement.hpp"
#include "URI.hpp"
#include "HTTPMethod.hpp"

struct HTTPFirstLine : public HTTPElement 
{
    std::string raw;
    std::string protocol;
    URI uri;
    HTTPMethod method;

    HTTPFirstLine();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & str);
   // void validate() const {std::cout << "Validated First Line. ";};
};

std::ostream & operator<<(std::ostream & os,  HTTPFirstLine const & line);