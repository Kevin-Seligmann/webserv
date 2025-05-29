#pragma once

#include <vector>
#include "HTTPElement.hpp"
#include "HTTPMethod.hpp"
#include "HTTPHeader.hpp"
#include "URI.hpp"
#include "Logger.hpp"

struct HTTPRequest : public HTTPElement
{
    HTTPMethod method;
    std::vector<HTTPHeader> headers; 
    URI uri;
    std::string protocol;
    // Body
    
    HTTPRequest();
    void reset();
    void print(std::ostream & os) const;
};

std::ostream & operator<<(std::ostream & os, HTTPRequest request);
