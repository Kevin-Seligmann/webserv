#include "Status.hpp"

namespace status
{
std::string const & status_to_text(Status const & status) 
{
    static const std::string ok = "Ok (200)";
    static const std::string bad_request = "Bad Request (400)";
    static const std::string content_too_large = "Content Too Large (413)";
    static const std::string uri_too_long = "URI Too Long (414)";
    static const std::string expectation_failed = "Expectation failed (417)";
    static const std::string unsupported_media_type = "Unsupported Media-Type (415)";
    static const std::string request_header_too_large = "Request Header Fields Too Large (431)";
    static const std::string not_implemented = "Not Implemented (501)";
    static const std::string version_not_supported = "HTTP Version Not Supported (505)";
    static const std::string unknown = "UNKNOWN STATUS";

    switch (status)
    {
        case OK: return ok;
        case BAD_REQUEST: return bad_request;
        case CONTENT_TOO_LARGE: return content_too_large;
        case URI_TOO_LONG: return uri_too_long;
        case REQUEST_HEADER_FIELDS_TOO_LARGE: return request_header_too_large;
        case NOT_IMPLEMENTED: return not_implemented;
        case VERSION_NOT_SUPPORTED: return version_not_supported;
        case EXPECTATION_FAILED: return expectation_failed;
        case UNSUPPORTED_MEDIA_TYPE: return unsupported_media_type;
        default: return unknown;
    }
}

std::string const & stoa(Status const & status) 
{
    static const std::string ok = "200";
    static const std::string bad_request = "400";
    static const std::string content_too_large = "413";
    static const std::string uri_too_long = "414";
    static const std::string expectation_failed = "417";
    static const std::string unsupported_media_type = "415";
    static const std::string request_header_too_large = "431";
    static const std::string not_implemented = "501";
    static const std::string version_not_supported = "505";

    switch (status)
    {
        case OK: return ok;
        case BAD_REQUEST: return bad_request;
        case CONTENT_TOO_LARGE: return content_too_large;
        case URI_TOO_LONG: return uri_too_long;
        case REQUEST_HEADER_FIELDS_TOO_LARGE: return request_header_too_large;
        case NOT_IMPLEMENTED: return not_implemented;
        case VERSION_NOT_SUPPORTED: return version_not_supported;
        case EXPECTATION_FAILED: return expectation_failed;
        case UNSUPPORTED_MEDIA_TYPE: return unsupported_media_type;
        default: throw std::runtime_error("Trying to send an invalid status");
    }
}
}
