#pragma once

struct ActiveFileDescriptor {
    ActiveFileDescriptor(int fd, int mode): fd(fd), mode(mode){};
    int fd;
    int mode;
};