#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "HTTPElement.hpp"

struct HTTPHeader : public HTTPElement
{
    std::string raw;

    HTTPHeader();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & str);
    //void validate() const {std::cout << "Validated HEADER. " << name;};
};

std::ostream & operator<<(std::ostream & os,  HTTPHeader const & header);