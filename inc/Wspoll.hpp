#pragma once

#include <vector>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <deque>
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
    bool is_full();
    Wspoll_event const & operator[](int fd);

private:
    enum Wspoll_request_type 
    {
        WSPOLL_ADD, WSPOLL_DEL, WSPOLL_MOD
    };

    enum Wspoll_mark 
    {
        WSPOLL_MARK_DELETION = -1,
        WSPOLL_MARK_MOD = -2,
    };

    struct Wspoll_request
    {
        Wspoll_request(Wspoll_request_type type, int fd, int mode);
        Wspoll_request(Wspoll_request_type type, int fd);
        Wspoll_request_type type;
        int fd;
        int mode;
    };

    const static int POLLING_SIZE = 50000;
    const static int TIMEOUT = 1000;
    const static int WSPOLL_DEBUG = 0;

    struct pollfd * _fds; 
    std::deque<Wspoll::Wspoll_request> _requests;
    int _size;
    int _adding_queue_size;

    void update_fds();
    void real_add(int fd, int mode);
    void real_mod(int fd, int mode);
    void real_del(int fd);

    Wspoll(Wspoll &);
    Wspoll & operator=(Wspoll &);
};