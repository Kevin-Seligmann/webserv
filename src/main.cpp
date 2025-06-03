#include "../inc/ServersManager.hpp"
#include "../inc/SocketsManager.hpp"
#include "../inc/Parsed.hpp"
#include "../inc/Utils.hpp"
#include "../inc/debug.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ParsedServers config;
        if (parseProcess(argc, argv, config) != 0)return (1);
        showParse(config);

        ServersManager webSevers;
        serversInit(webSevers, config);
        showServers(webSevers);

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
