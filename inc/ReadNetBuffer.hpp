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
    void append(std::string const & str);
    void expand(size_t min_size);
    void clear();
    void shrink();
    void reserve(ssize_t n);
    void consume_bytes(ssize_t n);

    ssize_t capacity() const;
    ssize_t size() const;

    std::string::iterator begin();
    std::string::iterator end();
    std::string::const_iterator cbegin() const;
    std::string::const_iterator cend() const;

    uint8_t * itbegin();
    uint8_t * itend();
    uint8_t const * citbegin() const;
    uint8_t const * citend() const;


    uint8_t * get_start();

protected:
    void discard_current();
    void manual_increase(size_t n);

private:
    static const size_t START_BUFFER_SIZE;
    static const size_t SHRINK_BUFFER_SIZE;

    ReadNetBuffer(ReadNetBuffer &);
    ReadNetBuffer & operator=(ReadNetBuffer &);

    
    uint8_t *_buffer;
    uint8_t *_start;
    uint8_t *_end;
    uint8_t *_tail;
};