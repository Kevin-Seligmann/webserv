#pragma once

#include <string>
#include <iostream>
#include "HTTPElement.hpp"

struct URI : public HTTPElement
{
    std::string raw;
    std::string host;
    int port;

    URI();
    void reset();
    void print(std::ostream & os) const;
    //void validate() const {std::cout << "Validated URI. ";};
};

std::ostream & operator<<(std::ostream & os, URI const & uri);