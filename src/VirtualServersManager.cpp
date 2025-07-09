#include "VirtualServersManager.hpp"
#include <iostream>

VirtualServersManager::VirtualServersManager() {
    // Constructor por defecto
}

VirtualServersManager::VirtualServersManager(const std::vector<ParsedServer>& configs) {
    for (std::vector<ParsedServer>::const_iterator it = configs.begin(); 
         it != configs.end(); ++it) {
        try {
            _servers.push_back(Server(*it));
        } catch (const std::exception& e) {
            std::cerr << "Error creating server: " << e.what() << std::endl;
        }
    }
}

VirtualServersManager::~VirtualServersManager() {
    // Cleanup resources if needed
}

void VirtualServersManager::run() {
    
    
    std::cout << "VirtualServersManager::run() - Event loop not implemented yet" << std::endl;
}

void VirtualServersManager::setupPollFDs() {
    _poll_fds.clear(); // para restarts
    for (size_t i = 0; i < _servers.size(); ++i) {
        pollfd server_pfd;
        server_pfd.fd = _servers[i].getSocketFD();
        server_pfd.events = POLLIN;
        server_pfd.revents = 0;
    }
}