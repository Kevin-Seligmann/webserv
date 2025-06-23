#pragma once

#include <string>
#include <vector>
#include <iostream>

struct MediaType 
{
    static 
    std::string type;
    std::string subtype;
    std::vector<std::pair<std::string, std::string> > parameters;

    void reset();
};

std::ostream & operator<<(std::ostream & os,  MediaType const & header);