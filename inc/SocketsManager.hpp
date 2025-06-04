#ifndef SOCKETSMANAGER_HPP
#define SOCKETSMANAGER_HPP

#include <map>
#include <string>
#include "Listen.hpp"

#define MAX_EVENTS 1024

struct SocketsManager {
    std::map<Listen, int> listenSockets;

    SocketsManager();
    ~SocketsManager();

    // FUNCTIONS
    
    bool isCreated(const Listen& pl);
};

#endif