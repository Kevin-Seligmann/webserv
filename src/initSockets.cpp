#include "../inc/SocketsManager.hpp"
#include "../inc/ServersManager.hpp"
#include "../inc/HostPort.hpp"
#include "../inc/Utils.hpp"
#include <iostream>

bool socketsInit(SocketsManager listenSockets, ServersManager webServers) {

    std::map<HostPort, std::vector<Servers> >::iterator it = webServers.serversManager.begin();
    for (; it != webServers.serversManager.end(); ++it) {
        if (!listenSockets.isCreated((*it).first)) {
            // crear el socket aqui
        }
        else {
            // qué hacer si ya está? nada? 
        }
        // creo sockets
        // configuro sockets
        // bindeo
        // valido que nada falle con los retornos de control

        // si está ok lo pongo en la estructura listenSockets
    }

    // nuevo bucle para hacer que todos lo sockets pasen a listen
    // crear estructura inversa dentro de listen sockets... que sea un mapa fd to HostPort
        // con el fd se llega a HostPort y con esto servirá para llegar a vector Servers
    
    std::cout << BLUE << "Success: sockets initialized." << RESET << std::endl;
    return true;
}