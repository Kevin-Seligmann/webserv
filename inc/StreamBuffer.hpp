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
    void set_external_buffer();
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












// #pragma once

// #include <stdint.h>
// #include <string>
// #include <stdexcept>
// #include <cstring>
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <unistd.h>

// class StreamBuffer 
// {

// public:
//     const static int STREAM_BUFFER_SIZE = 40000;

//     StreamBuffer();

//     ssize_t write();
//     ssize_t read();
//     ssize_t send();
//     ssize_t recv();

//     void set_write_end(int fd);
//     void set_read_end(int fd);
//     void reset();

// private:
//     uint8_t _buffer[STREAM_BUFFER_SIZE];
//     uint8_t * _head;
//     uint8_t * _tail;

//     int _write_fd;
//     int _read_fd;

//     size_t write_max_size() const;
//     size_t read_max_size() const;
//     uint8_t * write_ptr() const;
//     uint8_t * read_ptr() const;

//     void adjust_pointers_after_read(size_t bytes);
//     void adjust_pointers_after_write(size_t bytes);
// };