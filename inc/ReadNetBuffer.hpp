#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>

class ReadNetBuffer
{
public:
    ReadNetBuffer();
    ~ReadNetBuffer();

    void append(uint8_t const * str, ssize_t size);
    void expand(size_t min_size);
    void shrink();
    void consume_bytes(ssize_t n);

    ssize_t capacity() const;
    ssize_t size() const;
    
    std::string::iterator begin();
    std::string::iterator end();
    std::string::const_iterator cbegin() const;
    std::string::const_iterator cend() const;

protected:
    void discard_current();

private:
    static const size_t START_BUFFER_SIZE;
    static const size_t SHRINK_BUFFER_SIZE;

    uint8_t *_buffer;
    uint8_t *_start;
    uint8_t *_end;
    uint8_t *_tail;

    ReadNetBuffer(ReadNetBuffer &);
    ReadNetBuffer & operator=(ReadNetBuffer &);
};