#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include "HTTPElement.hpp"
#include "MediaType.hpp"

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
    std::vector<std::string> connections;
    std::vector<std::string> expectations;
    MediaType content_type;
    std::map<std::string, std::string> cookies;

    FieldSection();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & name, std::string const & value);
    void put_cookie(std::string::iterator name_start, std::string::iterator name_end, std::string::iterator value_start, std::string::iterator value_end);
};

std::ostream & operator<<(std::ostream & os,  FieldSection const & media);