#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include "HTTPElement.hpp"

struct FieldSection : public HTTPElement
{
    std::map<std::string, std::vector<std::string> > fields;

    std::string _raw;

    FieldSection();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & name, std::string const & value);
};

std::ostream & operator<<(std::ostream & os,  FieldSection const & header);