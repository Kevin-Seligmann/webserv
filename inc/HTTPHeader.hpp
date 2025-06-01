#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include "HTTPElement.hpp"

struct HTTPHeader : public HTTPElement
{
    std::map<std::string, std::string> fields;
    std::string _raw;

    HTTPHeader();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & str);
};

std::ostream & operator<<(std::ostream & os,  HTTPHeader const & header);