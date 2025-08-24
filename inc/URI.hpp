#pragma once

#include <string>
#include <iostream>
#include "HTTPElement.hpp"

struct URI : public HTTPElement
{
    // http://localhost:8080/cgipath/script.php/algo/otra_cosa?a=b&c=d#fragment
    std::string host; // 8080
    std::string path; // /cgipath/script.php/algo/otra_cosa
    std::string query; // ?a=b&c=d
    std::string fragment; // #fragment
    std::string schema; // http
    int port;
    
    size_t length;

    URI();
    void reset();
    void print(std::ostream & os) const;
};

std::ostream & operator<<(std::ostream & os, URI const & uri);