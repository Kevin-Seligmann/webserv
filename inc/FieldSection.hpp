#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include "HTTPElement.hpp"
#include "MediaType.hpp"

struct Coding 
{
    std::string name;
    std::vector<std::pair<std::string, std::string> > parameters;
};

enum e_close_status {RCS_CLOSE, RCS_KEEP_ALIVE};

struct FieldSection : public HTTPElement
{
    std::map<std::string, std::string > fields;

    std::string host;
    int port;
    int content_length;
    std::vector<Coding> transfer_encodings;
    std::vector<std::string> connections;
    std::vector<std::string> expectations;
    MediaType content_type;
    std::map<std::string, std::string> cookies;
    
    e_close_status close_status;

    FieldSection();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & name, std::string const & value);
    std::string const getContentType() const;
    void put_cookie(std::string::iterator name_start, std::string::iterator name_end, std::string::iterator value_start, std::string::iterator value_end);
};

std::ostream & operator<<(std::ostream & os,  FieldSection const & media);