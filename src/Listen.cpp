#include "../inc/Listen.hpp"
#include <sstream>

Listen::Listen(const std::string& host, const int port, bool is_default)
    : host(host), port(port), is_default(is_default) {}

Listen::~Listen() {}

std::string Listen::to_string() const {
    std::ostringstream oss;
    oss << host << ":" << port;
    if (is_default) oss << " [default_server]";
    return oss.str();
}

bool Listen::operator==(const Listen& other) const {
    return (host == other.host && port == other.port && is_default == other.is_default);
}

bool Listen::operator<(const Listen& other) const {
    if (host < other.host)
        return true;
    if (host > other.host)
        return false;
    if (port < other.port)
        return true;
    if (port > other.port)
        return false;
    return is_default < other.is_default;
}

std::ostream& operator<<(std::ostream& os, const Listen& pl) {
    os << pl.host << ":" << pl.port;
    return os;
}
