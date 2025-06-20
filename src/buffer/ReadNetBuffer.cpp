#include "ReadNetBuffer.hpp"

const size_t ReadNetBuffer::START_BUFFER_SIZE = 1000;
const size_t ReadNetBuffer::SHRINK_BUFFER_SIZE = 50000;

ReadNetBuffer::ReadNetBuffer()
{
    _buffer = new uint8_t[START_BUFFER_SIZE];
    _start = _buffer;
    _prev_start = _buffer;
    _tail = _buffer;
    _end = _buffer + START_BUFFER_SIZE;
}

ReadNetBuffer::~ReadNetBuffer(){delete [] _buffer;}

void ReadNetBuffer::discard_current()
{
    if (capacity() >= SHRINK_BUFFER_SIZE)
        shrink();
    _start = _tail;
    _prev_start = _start;
}

#include <iostream>

void ReadNetBuffer::append(uint8_t const * str, ssize_t size)
{
    if (this->size() + size > capacity())  
        expand();
    std::memcpy(_tail, str, size);
    _tail += size;
}

void ReadNetBuffer::shrink()
{
    uint8_t * new_buffer = new uint8_t[START_BUFFER_SIZE];
    delete [] _buffer;
    _buffer = new_buffer;
    _start = new_buffer;
    _tail = new_buffer;
    _prev_start = new_buffer;
    _end = new_buffer + START_BUFFER_SIZE;
}

void ReadNetBuffer::expand()
{
    size_t new_capacity = std::max<size_t>(capacity() * 1.7, START_BUFFER_SIZE);
    size_t size = this->size();
    size_t put_length = put_back_length();

    uint8_t * new_buffer = new uint8_t[new_capacity];
    std::memcpy(new_buffer, _prev_start, size + put_length);
    delete [] _buffer;
    _buffer = new_buffer;
    _prev_start = new_buffer;
    _start = new_buffer + put_length;
    _tail = _start + size;
    _end = new_buffer + new_capacity;
}

void ReadNetBuffer::consume_bytes(ssize_t bytes)
{
    _prev_start = _start;
    _start += bytes;
}

ssize_t ReadNetBuffer::put_back_length() const {return _start - _prev_start;}

ssize_t ReadNetBuffer::capacity() const {return _end - _buffer;}

ssize_t ReadNetBuffer::size() const {return _tail - _start;}

void ReadNetBuffer::put_back(){_start = _prev_start;}

std::string::iterator ReadNetBuffer::begin(){return std::string::iterator((char *) _start);}

std::string::iterator ReadNetBuffer::end(){return std::string::iterator((char *) _tail);}

std::string::const_iterator ReadNetBuffer::cbegin() const {return std::string::const_iterator((char *) _start);}

std::string::const_iterator ReadNetBuffer::cend() const{return std::string::const_iterator((char *) _tail);}