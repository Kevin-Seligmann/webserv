#include "../inc/SocketsManager.hpp"

SocketsManager::SocketsManager() {}

SocketsManager::~SocketsManager() {}

bool SocketsManager::isCreated(const HostPort& hp) {
    std::map<HostPort, int>::iterator it = listenSockets.begin();
    for (; it != listenSockets.end(); ++it) {
        if ((*it).first == hp) {
            return true;
        }
    }
    return false;
}