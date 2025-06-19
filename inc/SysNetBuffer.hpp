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


