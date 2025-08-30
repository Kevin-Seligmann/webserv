#pragma once

#include <vector>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <queue>
#include <cstring>
#include "Logger.hpp"
#include "StringUtil.hpp"

struct Wspoll_event 
{
    int fd;
    int events;
};

class Wspoll 
{
public:
    Wspoll();
    ~Wspoll();

    void add(int fd, int mode);
    void mod(int fd, int mode);
    void del(int fd);
    int wait();
    int size();
    Wspoll_event const & operator[](int fd);

private:
    enum Wspoll_request_type 
    {
        WSPOLL_ADD, WSPOLL_DEL, WSPOLL_MOD
    };

    struct Wspoll_request
    {
        Wspoll_request(Wspoll_request_type type, int fd, int mode);
        Wspoll_request(Wspoll_request_type type, int fd);
        Wspoll_request_type type;
        int fd;
        int mode;
    };

    const static int POLLING_SIZE = 64;
    const static int TIMEOUT = 1000;
    const static int WSPOLL_DEBUG = 0;

    struct pollfd * _fds;
    std::queue<Wspoll::Wspoll_request> _requests;
    int _size;

    void update_fds();
    void real_add(int fd, int mode);
    void real_mod(int fd, int mode);
    void real_del(int fd);

    Wspoll(Wspoll &);
    Wspoll & operator=(Wspoll &);
};