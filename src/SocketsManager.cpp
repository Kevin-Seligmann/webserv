#include "../inc/SocketsManager.hpp"

SocketsManager::SocketsManager() {}

SocketsManager::~SocketsManager() {}

bool SocketsManager::isCreated(const Listen& pl) {
    std::map<Listen, int>::iterator it = listenSockets.begin();
    for (; it != listenSockets.end(); ++it) {
        if ((*it).first == pl) {
            return true;
        }
    }
    return false;
}