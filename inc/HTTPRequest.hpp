#pragma once

#include <vector>
#include "HTTPElement.hpp"
#include "FieldSection.hpp"
#include "HTTPBody.hpp"
#include "Logger.hpp"

struct HTTPRequest : public HTTPElement
{
    URI uri;
    std::string protocol;
    HTTPMethod method;
    FieldSection headers; 
    HTTPBody body;
    
    HTTPRequest();
    std::string to_string();
    void reset();
    void print(std::ostream & os) const;
};

std::ostream & operator<<(std::ostream & os, HTTPRequest request);
