#pragma once

#include "SysBuffer.hpp"
#include <unistd.h>

class SysFileBuffer : public SysBuffer 
{
public:
    SysFileBuffer(int fd);
    ssize_t write(uint8_t *buff, size_t size);
    ssize_t read(uint8_t *buff, size_t size);

private:
};

SysFileBuffer::SysFileBuffer(int fd)
:SysBuffer(fd){}

ssize_t SysFileBuffer::write(uint8_t *buff, size_t size)
{
    return ::write(_fd, buff, size);
}

ssize_t SysFileBuffer::read(uint8_t *buff, size_t size)
{
    return ::read(_fd, buff, size);
}