#include "HTTPRequestBuffer.hpp"

HTTPRequestBuffer::HTTPRequestBuffer(){_offset = 0;}

HTTPRequestBuffer::~HTTPRequestBuffer(){}

void HTTPRequestBuffer::new_request() {discard_current(); _offset = 0; _last_read_size = 0;}

bool HTTPRequestBuffer::get_crlf_line(std::string::iterator & _begin, std::string::iterator & _end)
{
    std::string::iterator start = begin();
    std::string::iterator ptr = start + _offset;

    for (; ptr != end(); ++ptr) 
    {
        if (*ptr == '\n') 
        {
            if (ptr != start && *(ptr - 1) == '\r')
            {
                _begin = start;
                _end = ptr - 1;
            }
            else
            {
                _begin = start;
                _end = ptr;
            }
            _last_read_size = ptr - begin();       
            _offset = 0;
            consume_bytes(ptr - start + 1);
            return true;
        }
    };
    _offset = ptr - begin();
    _last_read_size = _offset;
    return false; 
}

// bool HTTPRequestBuffer::get_crlf_line(std::string & dst)
// {
//     std::string::iterator start = begin();
//     std::string::iterator ptr = start + _offset;

//     for (; ptr != end(); ++ptr) 
//     {
//         if (*ptr == '\n') 
//         {
//             if (ptr != start && *(ptr - 1) == '\r')
//                 dst.assign(start, ptr - 1);
//             else
//                 dst.assign(start, ptr );
//             _last_read_size = ptr - begin();       
//             _offset = 0;
//             consume_bytes(ptr - start + 1);
//             return true;
//         }
//     };
//     _offset = ptr - begin();
//     _last_read_size = _offset;
//     return false;
// }

// bool HTTPRequestBuffer::get_chunk(ssize_t chunk_size, std::string & dst)
// {
//     if (size() < chunk_size)
//     {
//         _last_read_size = size();
//         return false;
//     }
//     dst.assign(begin(), begin() + chunk_size);
//     consume_bytes(chunk_size);
//     _last_read_size = chunk_size;
//     return true;
// }

bool HTTPRequestBuffer::get_chunk(ssize_t chunk_size, std::string::iterator & _begin, std::string::iterator & _end)
{
    if (size() < chunk_size)
    {
        _last_read_size = size();
        return false;
    }
    _begin = begin();
    _end = begin() + chunk_size;
    consume_bytes(chunk_size);
    _last_read_size = chunk_size;
    return true;
}

ssize_t HTTPRequestBuffer::previous_read_size() const {return _last_read_size;}
