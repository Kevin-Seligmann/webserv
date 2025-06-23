#pragma once

#include <string>

enum Status {
    NO_STATUS = 0,
    OK = 200,
    BAD_REQUEST = 400,
    CONTENT_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    EXPECTATION_FAILED = 417,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    NOT_IMPLEMENTED = 501,
    VERSION_NOT_SUPPORTED = 505,
};

namespace status {
    std::string const & status_to_text(Status const & status);
}

