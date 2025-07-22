#pragma once

#include <stdint.h>
#include <sys/types.h>


class SysBuffer 
{
public:
    SysBuffer(int fd):_fd(fd){}
    
    virtual ~SysBuffer(){};
    virtual ssize_t read(uint8_t *buff, size_t size) = 0;
    virtual ssize_t write(uint8_t *buff, size_t size) = 0;

    int _fd;
};