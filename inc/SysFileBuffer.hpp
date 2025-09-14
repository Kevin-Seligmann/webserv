#pragma once

#include "SysBuffer.hpp"
#include <unistd.h>

class SysFileBuffer : public SysBuffer 
{
public:
    SysFileBuffer(int fd);
    ~SysFileBuffer();
    ssize_t write(uint8_t *buff, size_t size);
    ssize_t read(uint8_t *buff, size_t size);

private:
};
