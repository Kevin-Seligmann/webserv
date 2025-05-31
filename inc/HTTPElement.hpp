#pragma once

#include <iostream>

class HTTPElement 
{
public:
    virtual ~HTTPElement(){};
    virtual void print(std::ostream & os) const = 0;
    virtual void reset() = 0;
    //virtual void validate() = 0;

    // setConfig()
    
    // OR

    // virtual void validate(config *)
};