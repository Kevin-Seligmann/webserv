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
    std::string name;

    RequestParseTest(const char * input, Status status, std::string const & test_name)
        :input_string(input), expected_status(status), obtained_status(NO_STATUS), name(test_name)
        {}
};