#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Logger.hpp"
#include "StringUtil.hpp"

struct MediaType;

typedef std::vector<std::pair<MediaType, std::vector<std::string> > > t_mime_conf;

struct MediaType 
{
    static t_mime_conf ACCEPTED_TYPES;
    static const std::string TYPES_PATH;
    static void load_types();

    MediaType();
    MediaType(std::string type, std::string subtype);
    std::string type;
    std::string subtype;
    std::vector<std::pair<std::string, std::string> > parameters;

    static std::string filename_to_type(std::string const & filename);
    const std::string getString() const;

    void reset();
};

std::ostream & operator<<(std::ostream & os,  MediaType const & header);