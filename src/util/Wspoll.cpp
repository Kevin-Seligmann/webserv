#include "Wspoll.hpp"

Wspoll::Wspoll_request::Wspoll_request(Wspoll_request_type type, int fd, int mode)
:type(type), fd(fd), mode(mode){}

Wspoll::Wspoll_request::Wspoll_request(Wspoll_request_type type, int fd)
:type(type), fd(fd){}

Wspoll::Wspoll()
:_fds(NULL),
_size(0),
_adding_queue_size(0)
{
    _fds = new struct pollfd[POLLING_SIZE];
    for (int i = 0; i < POLLING_SIZE; i ++)
        _fds[i].fd = -1;
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll created" << std::endl;
}

Wspoll::~Wspoll()
{
    if (_fds)
        delete []_fds;
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll destroyed"  << std::endl;
}

int Wspoll::size(){return _size;}

void Wspoll::add(int fd, int mode)
{
    if (fd == -1)
        return ;
    if (_adding_queue_size + _size >= POLLING_SIZE)
        throw std::runtime_error("Wspoll failed: No space left on addition" + wss::i_to_dec(_adding_queue_size) + " + " +  wss::i_to_dec(_size)  + "/" + wss::i_to_dec(POLLING_SIZE));
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll addition requested " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
    _adding_queue_size ++;
    _requests.push_back(Wspoll_request(WSPOLL_ADD, fd, mode));
}

void Wspoll::mod(int fd, int mode)
{
    if (fd == -1)
        return ;
    for (int i = 0; i < _size; i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events = WSPOLL_MARK_MOD;
            break ;
        }
    }
    _requests.push_back(Wspoll_request(WSPOLL_MOD, fd, mode));
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll modification requested " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
}

void Wspoll::del(int fd)
{
    if (fd == -1)
        return ;
    for (int i = 0; i < _size; i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events = WSPOLL_MARK_DELETION;
            break ;
        }
    }
    _adding_queue_size --;
    _requests.push_front(Wspoll_request(WSPOLL_DEL, fd));
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll deletion requested " + wss::i_to_dec(fd) << std::endl;
}

void Wspoll::real_add(int fd, int mode)
{
    if (_size >= POLLING_SIZE)
        throw std::runtime_error("File descriptors exceed server capacity: " + wss::i_to_dec(_size) + "/" + wss::i_to_dec(POLLING_SIZE));
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll addition " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
    _fds[_size].events = mode;
    _fds[_size].fd = fd;
    _size ++;
}

void Wspoll::real_mod(int fd, int mode)
{
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll modification " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
    for (int i = 0; i < _size; i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events = mode;
            if (WSPOLL_DEBUG)
                Logger::getInstance() << "Wspoll modified " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
            return ;
        }
    }
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll modification didn't find the fd " + wss::i_to_dec(fd) + " " + wss::i_to_dec(mode) << std::endl;
}

void Wspoll::real_del(int fd)
{
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll removal requested " + wss::i_to_dec(fd) << std::endl;
    for (int i = 0; i < _size; i++)
    {
        if (_fds[i].fd == fd)
        {
            if (i < _size - 1)
                _fds[i] = _fds[_size - 1];
            _size --;
            if (WSPOLL_DEBUG)
                Logger::getInstance() << "Wspoll removal " + wss::i_to_dec(fd) << std::endl;
            return ;
        }
    }
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll removal didn't find fd " + wss::i_to_dec(fd) << std::endl;
}

void Wspoll::update_fds()
{
    _adding_queue_size = 0;
    while (!_requests.empty())
    {
        Wspoll_request & _request = _requests.front(); 
        switch (_request.type)
        {
            case WSPOLL_ADD: real_add(_request.fd, _request.mode); break;
            case WSPOLL_DEL: real_del(_request.fd); break ;
            case WSPOLL_MOD: real_mod(_request.fd, _request.mode); break ;
        }
        _requests.pop_front();
    }
}

int Wspoll::wait()
{
    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll waiting requested." << std::endl;
    update_fds();
    return poll(_fds, _size, TIMEOUT);
}

Wspoll_event const & Wspoll::operator[](int index)
{
    static Wspoll_event event;


    if (index < 0 || index >= _size)
        throw std::out_of_range("Wspoll index out of range");

    if (WSPOLL_DEBUG)
        Logger::getInstance() << "Wspoll accesed element on index " << index << ". Fd: " << _fds[index].fd << std::endl;

    event.fd = _fds[index].fd;
    if (_fds[index].events == WSPOLL_MARK_DELETION || _fds[index].events == WSPOLL_MARK_MOD)
        event.events = 0;
    else
        event.events = _fds[index].revents;
    return event;
}

bool Wspoll::is_full(){return _size == POLLING_SIZE;}