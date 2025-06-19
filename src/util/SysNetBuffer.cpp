#include "SysNetBuffer.hpp"

SysNetBuffer::SysNetBuffer(int socket_fd):SysBuffer(socket_fd){}

ssize_t SysNetBuffer::write(uint8_t *buff, size_t size)
{
    return send(_fd, buff, size, 0);
}

ssize_t SysNetBuffer::read(uint8_t *buff, size_t size)
{
    return recv(_fd, buff, size, 0);
}