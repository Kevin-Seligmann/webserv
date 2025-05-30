#include "../inc/HostPort.hpp"

HostPort::HostPort(const std::string& host, const int port)
    : host(host), port(port) {}

HostPort::~HostPort() {}

bool HostPort::operator==(const HostPort& other) const {
    return (host == other.host && port == other.port);
}

bool HostPort::operator<(const HostPort& other) const {
    if (host < other.host)
        return true;
    if (host > other.host)
        return false;
    return port < other.port;
}

std::ostream& operator<<(std::ostream& os, const HostPort& hp) {
    os << hp.host << ":" << hp.port;
    return os;
}
