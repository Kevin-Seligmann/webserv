#include "../inc/ServersManager.hpp"
#include "../inc/SocketsManager.hpp"
#include "../inc/Parsed.hpp"
#include "../inc/functions.hpp"
#include "../inc/Utils.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ParsedServers config;
        
        // Parse configuration file and validate arguments
        if (parseProcess(argc, argv, config) != 0) {
            return (1);
        }

        // estructura que tendrá los datos de los servers
        ServersManager webSevers;
        // inicialización de estructura con datos de los servers
        serversInit(webSevers, config);

        // check de las fases previas
        showServers(webSevers);

        // estructura para los fd de los sockets
        SocketsManager listenSockets;
        socketsInit(listenSockets, webSevers);

        return (0);
    }
    catch (const std::exception& e) {
        std::cerr << RED << "FATAL ERROR: " << RESET << e.what() << std::endl;
        return (1);
    }
    catch (...) {
        std::cerr << RED << "FATAL ERROR: " << RESET << "Unknown exception occurred" << std::endl;
        return (1);
    }
}
