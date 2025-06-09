#pragma once

#include "HTTPElement.hpp"
#include "URI.hpp"
#include "HTTPMethod.hpp"

struct HTTPBody : public HTTPElement 
{
    std::string content;

    HTTPBody();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & str);
   // void validate() const {std::cout << "Validated body. ";};
};

std::ostream & operator<<(std::ostream & os,  HTTPBody const & line);