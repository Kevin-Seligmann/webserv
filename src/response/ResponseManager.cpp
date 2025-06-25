#include "ResponseManager.hpp"

void ResponseManager::generate_response(HTTPRequest & request){}

void ResponseManager::process()
{
    ssize_t written_bytes = _sys_buffer->write(_buffer.get_start(), 1);
    if (written_bytes > 0)
    {
        _buffer.consume_bytes(written_bytes);
    }
}

bool ResponseManager::response_done(){return _buffer.size() == 0;}
