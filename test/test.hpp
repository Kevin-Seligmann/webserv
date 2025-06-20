#pragma once

#include <iostream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>
#include <stdint.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include "Status.hpp"

struct RequestParseTest 
{
    // Request
    const char *input_string;

    // Status
    Status expected_status;
    Status obtained_status;

    // Test name and description
    std::string filename;

    RequestParseTest(std::string const & filename):filename(filename){}
};