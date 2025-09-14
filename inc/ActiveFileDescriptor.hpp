#pragma once

struct ActiveFileDescriptor {
    ActiveFileDescriptor(int fd, int mode): fd(fd), mode(mode){};
    
    ActiveFileDescriptor & operator=(ActiveFileDescriptor const & rhs)
    {
        fd = rhs.fd;
        mode = rhs.mode;
        return *this;
    }
    
    bool operator==(ActiveFileDescriptor const & rhs) const 
    {
        return fd == rhs.fd && mode == rhs.mode;
    }

    bool operator!=(ActiveFileDescriptor const & rhs) const 
    {
        return !(*this == rhs);
    }

    int fd;
    int mode;
};