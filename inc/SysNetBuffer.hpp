#pragma once

#include "SysBuffer.hpp"
#include <sys/socket.h>

class SysNetBuffer : public SysBuffer 
{
public:
    SysNetBuffer(int socket_fd);
    ssize_t write(uint8_t *buff, size_t size);
    ssize_t read(uint8_t *buff, size_t size);

private:
};


SysNetBuffer::SysNetBuffer(int socket_fd)
:SysBuffer(socket_fd)
{
    // Make non blocking here, maybe
}

ssize_t SysNetBuffer::write(uint8_t *buff, size_t size)
{
    return send(_fd, buff, size, 0);
}

ssize_t SysNetBuffer::read(uint8_t *buff, size_t size)
{
    return recv(_fd, buff, size, 0);
}