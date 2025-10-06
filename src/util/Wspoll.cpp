#include "Wspoll.hpp"
#include <cassert>
#include <fcntl.h>

Wspoll::Wspoll_request::Wspoll_request(Wspoll_request_type type, int fd, int mode)
:type(type), fd(fd), mode(mode){}

Wspoll::Wspoll_request::Wspoll_request(Wspoll_request_type type, int fd)
:type(type), fd(fd){}

Wspoll::Wspoll()
{
    for (int i = 0; i < POLLING_SIZE; i ++)
    {
        _fds[i].fd = -1;
        _fds[i].events = 0;
        _fds[i].revents = 0;
    }
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll created" << std::endl;
}

Wspoll::~Wspoll()
{
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll destroyed"  << std::endl;
}

int Wspoll::size(){return POLLING_SIZE;}

void Wspoll::add(int fd, int mode)
{
    if (fd == -1)
        return ;
    for (int i = 0; i < POLLING_SIZE; i++)
    {
        if (_fds[i].fd == fd)
        {
            mod(fd, mode);
            return ;
        }
    }
    for (int i = 0; i < POLLING_SIZE; i++)
    {
        if (_fds[i].fd == -1)
        {
            _fds[i].fd = fd;
            _fds[i].events = mode;
            _fds[i].revents = 0;
            return ;
        }
    }
    throw std::runtime_error("Wspoll failed: No space left on addition");

    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll addition requested " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
}

void Wspoll::mod(int fd, int mode)
{
    if (fd == -1)
        return ;
    for (int i = 0; i < POLLING_SIZE; i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events = mode;
            _fds[i].revents = 0;
            return ;
        }
    }

    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll modification requested " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
}

void Wspoll::del(int fd)
{
    if (fd == -1)
        return ;
    for (int i = 0; i < POLLING_SIZE; i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events = 0;
            _fds[i].revents= 0;
            _fds[i].fd = -1;
        }
    }

    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll deletion requested " + wss::i_to_dec(fd) << std::endl;
}

int Wspoll::wait()
{
    return poll(_fds, POLLING_SIZE, TIMEOUT);
}

Wspoll_event const & Wspoll::operator[](int index)
{
    static Wspoll_event event;

    if (index < 0 || index >= POLLING_SIZE)
        throw std::out_of_range("Wspoll index out of range");

    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll accesed element on index " << index << ". Fd: " << _fds[index].fd << std::endl;

    event.fd = _fds[index].fd;
    event.events = _fds[index].revents;
    return event;
}
