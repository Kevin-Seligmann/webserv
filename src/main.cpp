#include "../inc/ServersManager.hpp"
#include "../inc/functions.hpp"
#include "../inc/colors.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>

void choseParsingPath(int ac, char **av, ParsedServers& parsedConfig);
void showServers(ServersManager ws);

int main(int ac, char** av) {

    ParsedServers parsedConfig; // estructura que será receptora del parseo
    choseParsingPath(ac, av, parsedConfig); // donde hacemos el parse del default o el file que entra

    ServersManager ws;
    serversInit(ws, parsedConfig);

    showServers(ws);

    return 0;
}

// SIMPLE ARBOL DE CONDICIONES QUE EVALUA ARGUMENTOS, PARA SACARLO DEL FLUJO DEL MAIN
void choseParsingPath(int ac, char **av, ParsedServers& parsedConfig) {
    (void)av;
    if (ac == 1) {
        fakeConfig(parsedConfig); // crea fake config para dev
        // parseDefaultConfigFile(parsedConfig); DESCOMENTAR AL MERGEAR CON EL PARSEO
        std::cout << RED << "Warning: starting server with default config" << RESET << std::endl;
        std::cout << "No configuration file provided" << std::endl;
        std::cout << "Using default config file: ./conf/default.conf" << std::endl;
    }
    else if (ac == 2) {
        // parseConfigFile(av[1], parsedConfig); // pasa parsedConfig para actualizarlo
        std::cout << "Success: starting server with custom config" << std::endl;
    }
    else {
        std::cout << RED << "Error: too many arguments" << RESET << std::endl;
        exit (1);
    }
}


// SOLO PARA DEBUG Y VALIDAR QUE SE ESTA GUARDANDO OK
void showServers(ServersManager ws) {
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
}