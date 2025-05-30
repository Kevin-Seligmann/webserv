#pragma once

#include <string>
#include <iostream>
#include <cstdint>
#include "HTTPRequest.hpp"

class RequestParser 
{
public:
    RequestParser(HTTPRequest & request);

    void append(uint8_t *str, ssize_t size);
    
private:
    HTTPRequest & _request;

};