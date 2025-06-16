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
    void expand();
    void shrink();
    void put_back();
    void consume_bytes(ssize_t n);

    ssize_t capacity() const;
    ssize_t size() const;
    
    uint8_t * begin();
    uint8_t * end();
    const uint8_t * cbegin() const;
    const uint8_t * cend() const;

protected:
    void discard_current();

private:
    static const size_t START_BUFFER_SIZE;
    static const size_t SHRINK_BUFFER_SIZE;

    uint8_t *_buffer;
    uint8_t *_start;
    uint8_t *_end;
    uint8_t *_prev_start;
    uint8_t *_tail;

    ReadNetBuffer(ReadNetBuffer &);
    ReadNetBuffer & operator=(ReadNetBuffer &);

    ssize_t put_back_length() const;
};