#ifndef LISTEN_HPP
#define LISTEN_HPP

#include <string>
#include <iostream>

struct Listen {
    std::string host;
    int         port;
    bool        is_default;

    Listen(const std::string& host = "0.0.0.0", const int port = 8080, bool is_default = false);
    ~Listen();

    std::string to_string() const;

    bool operator==(const Listen& other) const;
    bool operator<(const Listen& other) const;
};

std::ostream& operator<<(std::ostream& os, const Listen& pl);

#endif
