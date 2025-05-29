#include "../inc/ServersManager.hpp"
#include "../inc/functions.hpp"
#include <iostream>
#include <sstream>

int main(int ac, char** av) {
    (void) ac; // casteo solo para dev
    (void) av; // casteo solo para dev

    ParsedServers parsedConfigFile; // estructura que será receptora del parseo
    fakeConfig(parsedConfigFile); // crea fake config para dev


    // al integrar el parseo se debe cambiar lo anterior por esto o similar...
    /*
    if ac == 1 {
        std::cout << "Warning: starting server with default config" << std::endl;
        useDefaultConfigFile(parsedConfigFile); // pasa parsedConfigFile para actualizarlo
    }
    else if ac == 2 {
        std::cout << "Success: starting server with custom config" << std::endl;
        parseConfigFile(av[1], parsedConfigFile); // pasa parsedConfigFile para actualizarlo
    }
    else
        std::cout << "Error: too many arguments" << std::endl;
    */

    ServersManager ws;
    serversInit(ws, parsedConfigFile);

    for (int i = 0; i < 17; ++i) {std::cout << "-";} std::cout << std::endl;
    std::cout << "Servers configured:" << std::endl;
    size_t i = 1;
    std::map<HostPort, std::vector<Servers> >::const_iterator ws_it;
    for (ws_it = ws.serversManager.begin(); ws_it != ws.serversManager.end(); ++ws_it) {
        const std::vector<Servers>& servers_vec = ws_it->second;
        std::vector<Servers>::const_iterator s_it;
        for (s_it = servers_vec.begin(); s_it != servers_vec.end(); ++s_it, ++i) {
            const Servers& srv = *s_it;
            std::ostringstream oss;
            oss << i;
            std::cout << "Server: " << oss.str() << std::endl;
            std::cout << "  Host: " << srv.getHost() << std::endl;
            std::cout << "  Port: " << srv.getPort() << std::endl;
            std::cout << "  Default: " << (srv.isDefaultServer() ? "yes" : "no") << std::endl;
            std::cout << "  Name: " << srv.getServerName() << std::endl;
        }
    }

    return 0;
}