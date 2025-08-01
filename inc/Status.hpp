#pragma once

#include <string>
#include <stdexcept>

enum Status {
    NO_STATUS = 0,
    OK = 200,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    CONTENT_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    EXPECTATION_FAILED = 417,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    VERSION_NOT_SUPPORTED = 505,
};

enum StatusType {
    STYPE_IMMEDIATE_RESPONSE,
    STYPE_GENERATE_RESPONSE
};

namespace status {
    std::string const & status_to_text(Status const & status);
    std::string const & stoa(Status const & status);
    StatusType status_type(Status const & status);
}

