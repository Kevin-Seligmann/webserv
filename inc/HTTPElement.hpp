#pragma once

#include <iostream>

class HTTPElement 
{
public:
    virtual ~HTTPElement() {};
    virtual void print(std::ostream & os) const = 0;
    virtual void reset() = 0;
};