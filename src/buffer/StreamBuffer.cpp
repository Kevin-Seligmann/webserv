#include "StreamBuffer.hpp"

StreamBuffer::StreamBuffer(){reset();}

void StreamBuffer::set_write_end(int fd){_write_fd = fd;}
void StreamBuffer::set_read_end(int fd){_read_fd = fd;}
void StreamBuffer::reset(){_size = 0;}
size_t StreamBuffer::size() const{ return _size;}

ssize_t StreamBuffer::write()
{
    ssize_t write_return = ::write(_write_fd, _buffer, size());
    if (write_return > 0)
        consume_bytes(write_return);
    return write_return; 
}

ssize_t StreamBuffer::read()
{
    size_t avaiable_space = STREAM_BUFFER_SIZE - size();
    ssize_t read_return = ::read(_read_fd, _buffer + size(), avaiable_space);
    if (read_return > 0)
        append_bytes(read_return);
    return read_return;
}

ssize_t StreamBuffer::send()
{
    ssize_t write_return = ::send(_write_fd, _buffer, size(), 0);
    if (write_return > 0)
        consume_bytes(write_return);
    return write_return; 
}

ssize_t StreamBuffer::recv()
{
    size_t avaiable_space = STREAM_BUFFER_SIZE - size();
    ssize_t read_return = ::recv(_read_fd, _buffer + size(), avaiable_space, 0);
    if (read_return > 0)
        append_bytes(read_return);
    return read_return;
}

void StreamBuffer::consume_bytes(size_t bytes)
{
    memmove(_buffer, _buffer + bytes, _size - bytes);
    _size -= bytes;
}

void StreamBuffer::append_bytes(size_t bytes)
{
    _size += bytes;
}


ssize_t StreamBuffer::write_external()
{
    ssize_t write_return = ::write(_write_fd, _external_buffer, _external_size);
    if (write_return > 0)
        consume_bytes_external(write_return);
    return write_return; 
}

ssize_t StreamBuffer::read_external(size_t max)
{
    size_t avaiable_space = _external_capacity - external_size();
    size_t read_size = std::min(max, avaiable_space);
    ssize_t read_return = ::read(_read_fd, _external_buffer + external_size(), read_size);
    if (read_return > 0)
        append_bytes_external(read_return);
    return read_return;
}

ssize_t StreamBuffer::send_external()
{
    ssize_t write_return = ::send(_write_fd, _external_buffer, _external_size, 0);
    if (write_return > 0)
        consume_bytes_external(write_return);
    return write_return; 
}
ssize_t StreamBuffer::recv_external(size_t max)
{
    size_t avaiable_space = _external_capacity - external_size();
    size_t read_size = std::min(max, avaiable_space);
    ssize_t read_return = ::recv(_read_fd, _external_buffer + external_size(), read_size, 0);
    if (read_return > 0)
        append_bytes_external(read_return);
    return read_return;

}

size_t StreamBuffer::external_size() const 
{
    return _external_size;
}

void StreamBuffer::set_external_buffer(uint8_t * buffer, ssize_t capacity, ssize_t size)
{
    _external_buffer = buffer;
    _external_capacity = capacity;
    _external_size = size;
}

void StreamBuffer::consume_bytes_external(size_t bytes)
{
    memmove(_external_buffer, _external_buffer + bytes, _external_size - bytes);
    _external_size -= bytes;
}

void StreamBuffer::append_bytes_external(size_t bytes)
{
    _external_size += bytes;
}


























// #include "StreamBuffer.hpp"

// StreamBuffer::StreamBuffer(){reset();}

// void StreamBuffer::set_write_end(int fd){_write_fd = fd;}
// void StreamBuffer::set_read_end(int fd){_read_fd = fd;}
// void StreamBuffer::reset(){_head = _buffer; _tail = _buffer;}

// size_t StreamBuffer::write_max_size() const 
// {
//     if (_tail >= _head)
//         return _tail - _head;
//     else
//         return _buffer + sizeof(_buffer) - _head;
// }

// size_t StreamBuffer::read_max_size() const 
// {
//     if (_tail >= _head)
//         return _buffer + sizeof(_buffer) - _tail;
//     else
//         return _head - _tail;
// }

// uint8_t * StreamBuffer::write_ptr() const
// {
//     return _head;
// }

// uint8_t * StreamBuffer::read_ptr() const
// {
//     return _tail;
// }

// void StreamBuffer::adjust_pointers_after_read(size_t bytes)
// {
//     if (_tail + bytes >= _buffer + sizeof(_buffer))
//         _tail = _buffer;
//     else 
//         _tail += bytes;
// }

// void StreamBuffer::adjust_pointers_after_write(size_t bytes)
// {
//     if (_head + bytes >= _buffer + sizeof(_buffer))
//         _head = _buffer;
//     else
//         _head += bytes;
// }


// ssize_t StreamBuffer::write()
// {
//     ssize_t write_return = ::write(_write_fd, write_ptr(), write_max_size());
//     if (write_return > 0)
//         adjust_pointers_after_write(write_return);
//     return write_return; 
// }

// ssize_t StreamBuffer::read()
// {
//     ssize_t read_return = ::read(_read_fd, read_ptr(), read_max_size());
//     if (read_return > 0)
//         adjust_pointers_after_read(read_return);
//     return read_return;
// }

// ssize_t StreamBuffer::send()
// {
//     ssize_t write_return = ::send(_write_fd, write_ptr(), write_max_size(), 0);
//     if (write_return > 0)
//         adjust_pointers_after_write(write_return);
//     return write_return; 
// }

// ssize_t StreamBuffer::recv()
// {
//     ssize_t read_return = ::recv(_read_fd, read_ptr(), read_max_size(), 0);
//     if (read_return > 0)
//         adjust_pointers_after_read(read_return);
//     return read_return;
// }