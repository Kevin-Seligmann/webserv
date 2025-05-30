#include "../inc/ServersManager.hpp"
#include "../inc/functions.hpp"
#include "../inc/colors.hpp"
#include <iostream>
#include <sstream>

int main(int ac, char** av) {
    (void) ac; // casteo solo para dev
    (void) av; // casteo solo para dev

    ParsedServers parsedConfigFile; // estructura que será receptora del parseo
    
    if (ac == 1) {
        fakeConfig(parsedConfigFile); // crea fake config para dev
        std::cout << RED << "Warning: starting server with default config" << RESET << std::endl;
        std::cout << "No configuration file provided" << std::endl;
        std::cout << "Using default config file: ./conf/default.conf" << std::endl;
//      useDefaultConfigFile(parsedConfigFile); // pasa parsedConfigFile para actualizarlo
    }
    else if (ac == 2) {
        // parseConfigFile(av[1], parsedConfigFile); // pasa parsedConfigFile para actualizarlo
        std::cout << "Success: starting server with custom config" << std::endl;
    }
    else
        std::cout << "Error: too many arguments" << std::endl;

    ServersManager ws;
    serversInit(ws, parsedConfigFile);

    for (int i = 0; i < 17; ++i) {std::cout << "-";} std::cout << std::endl;
    std::cout << GRN << "Servers configured:" << RESET << std::endl;
    size_t i = 1;
    std::map<HostPort, std::vector<Servers> >::const_iterator ws_it;
    for (ws_it = ws.serversManager.begin(); ws_it != ws.serversManager.end(); ++ws_it) {
        const std::vector<Servers>& servers_vec = ws_it->second;
        std::vector<Servers>::const_iterator s_it;
        for (s_it = servers_vec.begin(); s_it != servers_vec.end(); ++s_it, ++i) {
            const Servers& srv = *s_it;
            std::ostringstream oss;
            oss << i;
            std::cout << "Server (" << GRN << oss.str() << RESET << ")" << std::endl;
            std::cout << "  HostPort: " << srv.getHostPort() << std::endl;
            std::cout << "  Host: " << srv.getHost() << std::endl;
            std::cout << "  Port: " << srv.getPort() << std::endl;
            std::cout << "  Default: " << (srv.isDefaultServer() ? "yes" : "no") << std::endl;
            std::cout << "  Name: " << YEL << (srv.getServerName().empty() ? "Empty: no_name" : srv.getServerName()) << RESET << std::endl;
        }
    }

    return 0;
}