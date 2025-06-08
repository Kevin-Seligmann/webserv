#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include "HTTPElement.hpp"
#include "ErrorContainer.hpp"

struct FieldSection : public HTTPElement
{
    typedef std::map<std::string, std::vector<std::string> > field;
    field fields;

    std::string host;
    int port;
    int content_length;

    FieldSection();
    void reset();
    void print(std::ostream & os) const;
    void put(std::string const & name, std::string const & value);
    void process_headers(ErrorContainer & error_container);

};

std::ostream & operator<<(std::ostream & os,  FieldSection const & header);