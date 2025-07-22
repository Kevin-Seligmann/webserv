#pragma once

#include <vector>
#include <sstream>
#include "HTTPElement.hpp"
#include "FieldSection.hpp"
#include "HTTPBody.hpp"
#include "Logger.hpp"

struct HTTPRequest : public HTTPElement
{
    // Request componentes
    URI uri;
    std::string protocol;
    HTTPMethod method;
    FieldSection headers; 
    HTTPBody body;
    
    HTTPRequest();
    std::string to_string();
    void reset();
    void print(std::ostream & os) const;

    int get_port() const;
    std::string const get_host() const;
    std::string const get_path() const;
};

std::ostream & operator<<(std::ostream & os, const HTTPRequest &request);
