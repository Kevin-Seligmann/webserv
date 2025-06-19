#pragma once

#include <stdexcept>
#include "SysBuffer.hpp"
#include "SysNetBuffer.hpp"
#include "SysFileBuffer.hpp"

class SysBufferFactory 
{
public:
    enum sys_buffer_type {SYSBUFF_FILE, SYSBUFF_SOCKET};
    static SysBuffer * get_buffer(sys_buffer_type type, int fd);
    
private:
    SysBufferFactory();
};
