#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include "ReadNetBuffer.hpp"

class StreamBuffer 
{

public:
    const static int STREAM_BUFFER_SIZE = 40000;

    StreamBuffer();

    ssize_t write();
    ssize_t read();
    ssize_t send();
    ssize_t recv();


    void set_write_end(int fd);
    void set_read_end(int fd);
    void reset();
    size_t size() const;

    ssize_t write_external();
    ssize_t send_external();
    ssize_t recv_external(size_t max);
    ssize_t read_external(size_t max);

    size_t external_size() const;
    void set_external_buffer(uint8_t * buffer, ssize_t capacity, ssize_t size);

    int write_fd() {return _write_fd;};
    int read_fd() {return _read_fd;};

    ssize_t external_size(){return _external_size;};
    ssize_t external_capacity(){return _external_capacity;};
    ssize_t capacity(){return STREAM_BUFFER_SIZE;};

private:
    uint8_t _buffer[STREAM_BUFFER_SIZE];
    uint8_t * _external_buffer;
    int _write_fd;
    int _read_fd;
    size_t _size;
    size_t _external_size;
    size_t _external_capacity;

    void consume_bytes(size_t bytes);
    void append_bytes(size_t bytes);

    void consume_bytes_external(size_t bytes);
    void append_bytes_external(size_t bytes);

};