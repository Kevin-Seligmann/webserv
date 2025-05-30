#ifndef HOSTPORT_HPP
#define HOSTPORT_HPP

#include <string>
#include <iostream>

struct HostPort {
    std::string host;
    int         port;

    HostPort(const std::string& host, const int port);
    ~HostPort();

    bool operator==(const HostPort& other) const;
    bool operator<(const HostPort& other) const;
};

std::ostream& operator<<(std::ostream& os, const HostPort& hp);

#endif
