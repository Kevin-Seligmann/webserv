#include <iostream>
#include "ReadNetBuffer.hpp"
#include <cassert>

const size_t ReadNetBuffer::START_BUFFER_SIZE = 1000;
const size_t ReadNetBuffer::SHRINK_BUFFER_SIZE = 1001;

ReadNetBuffer::ReadNetBuffer()
{
    _buffer = new uint8_t[START_BUFFER_SIZE + 10000];
    _start = _buffer;
    _tail = _buffer;
    _end = _buffer + START_BUFFER_SIZE;
}

ReadNetBuffer::~ReadNetBuffer(){delete [] _buffer;}

void ReadNetBuffer::discard_current()
{
    if (capacity() >= SHRINK_BUFFER_SIZE)
        shrink();
}

void ReadNetBuffer::shrink()
{
    size_t size = this->size();
    size_t new_capacity = std::max<size_t>(START_BUFFER_SIZE, size);

    uint8_t * new_buffer = new uint8_t[new_capacity + 10000];
    memcpy(new_buffer, _start, size);
    delete [] _buffer;

    _buffer = new_buffer;
    _start = new_buffer;
    _tail = _start + size;
    _end = new_buffer + new_capacity;
}

void ReadNetBuffer::append(uint8_t const * str, ssize_t size)
{
    if (_tail - _buffer + size > capacity())
        expand(_tail - _buffer + size);
    std::memcpy(_tail, str, size);
    _tail += size;
}

void ReadNetBuffer::expand(size_t min_size)
{
    size_t new_capacity = std::max<size_t>(std::max<size_t>(capacity() * 1.7, START_BUFFER_SIZE), min_size);
    size_t size = this->size();

    uint8_t * new_buffer = new uint8_t[new_capacity + 10000];
    std::memcpy(new_buffer, _start, size);
    delete [] _buffer;

    _buffer = new_buffer;
    _start = new_buffer;
    _tail = _start + size;
    _end = new_buffer + new_capacity;
}

void ReadNetBuffer::consume_bytes(ssize_t bytes)
{
    _start += bytes;
}

ssize_t ReadNetBuffer::capacity() const {return _end - _buffer;}

ssize_t ReadNetBuffer::size() const {return _tail - _start;}

std::string::iterator ReadNetBuffer::begin(){return std::string::iterator((char *) _start);}

std::string::iterator ReadNetBuffer::end(){return std::string::iterator((char *) _tail);}

std::string::const_iterator ReadNetBuffer::cbegin() const {return std::string::const_iterator((char *) _start);}

std::string::const_iterator ReadNetBuffer::cend() const{return std::string::const_iterator((char *) _tail);}