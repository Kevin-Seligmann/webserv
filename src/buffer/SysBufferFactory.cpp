
#include "SysBufferFactory.hpp"

SysBuffer *  SysBufferFactory::get_buffer(sys_buffer_type type, int fd)
{
    switch (type)
    {
        case SYSBUFF_FILE: return new SysFileBuffer(fd);
        case SYSBUFF_SOCKET: return new SysNetBuffer(fd);
        default: throw new std::runtime_error("Code error: unknown SysBuffer type");
    }
}