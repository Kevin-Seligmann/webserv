#include "SysFileBuffer.hpp"

SysFileBuffer::SysFileBuffer(int fd)
:SysBuffer(fd){}

SysFileBuffer::~SysFileBuffer()
{
    // close(_fd);
}

ssize_t SysFileBuffer::write(uint8_t *buff, size_t size)
{
    return ::write(_fd, buff, size);
}

ssize_t SysFileBuffer::read(uint8_t *buff, size_t size)
{
    return ::read(_fd, buff, size);
}