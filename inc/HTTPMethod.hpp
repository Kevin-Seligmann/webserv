#pragma once

#include "iostream"

enum HTTPMethod 
{
    NOMETHOD, GET, POST, DELETE, PUT
};

std::ostream & operator<<(std::ostream & os, HTTPMethod method);
