#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "HTTPElement.hpp"

struct HTTPHeader : public HTTPElement
{
    std::string raw_header;
    std::string name;
    std::vector<std::string> values;

    HTTPHeader();
    void reset();
    void print(std::ostream & os) const;
};

std::ostream & operator<<(std::ostream & os,  HTTPHeader const & header);