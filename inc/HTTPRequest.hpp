#pragma once

#include <vector>
#include "HTTPElement.hpp"
#include "HTTPHeader.hpp"
#include "HTTPBody.hpp"
#include "Logger.hpp"
#include "HTTPFirstLine.hpp"

struct HTTPRequest : public HTTPElement
{
    HTTPFirstLine first_line;
    HTTPHeader headers; 
    HTTPBody body;
    
    HTTPRequest();
    void reset();
    void print(std::ostream & os) const;
    //void validate() const {std::cout << "Validated REQUEST. ";};
};

std::ostream & operator<<(std::ostream & os, HTTPRequest request);
