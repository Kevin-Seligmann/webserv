#ifndef SOCKETSMANAGER_HPP
#define SOCKETSMANAGER_HPP

#include <map>
#include <string>
#include "Listen.hpp"

struct SocketsManager {
    std::map<Listen, int> listenSockets;

    SocketsManager();
    ~SocketsManager();

    bool isCreated(const Listen& pl);
};

#endif