#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include "HTTPElement.hpp"

struct CommaSeparatedFieldValue 
{
    std::string name;
    std::vector<std::pair<std::string, std::string> > parameters;
};

struct FieldSection : public HTTPElement
{
    std::map<std::string, std::string > fields;

    std::string host;
    int port;
    int content_length;
    std::vector<CommaSeparatedFieldValue> transfer_encodings;

    FieldSection();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & name, std::string const & value);

};

std::ostream & operator<<(std::ostream & os,  FieldSection const & header);