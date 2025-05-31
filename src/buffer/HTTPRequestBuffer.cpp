#include "HTTPRequestBuffer.hpp"

HTTPRequestBuffer::HTTPRequestBuffer(){}

HTTPRequestBuffer::~HTTPRequestBuffer(){}

void HTTPRequestBuffer::new_request() {discard_current();}

bool HTTPRequestBuffer::get_crlf_line(std::string & dst)
{
    uint8_t * start = begin();
    uint8_t * ptr = start;

    for (; ptr != end(); ++ptr) {
        if (*ptr == '\n') 
        {
            if (ptr != start && *(ptr - 1) == '\r')
                dst.assign(start, ptr - 1);
            else
                dst.assign(start, ptr );
            consume_bytes(ptr - start + 1);
            return true;
        }
    };
    return false;
}
