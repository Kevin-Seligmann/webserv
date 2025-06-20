#pragma once

#include <string>
#include <iostream>
#include "HTTPElement.hpp"

struct URI : public HTTPElement
{
    std::string host;
    std::string path;
    std::string query;
    std::string fragment;
    std::string schema;
    int port;
    
    size_t length;

    URI();
    void reset();
    void print(std::ostream & os) const;
};

std::ostream & operator<<(std::ostream & os, URI const & uri);