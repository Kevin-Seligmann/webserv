#ifndef SOCKETINFO_HPP
#define SOCKETINFO_HPP

#include "VirtualServersManager.hpp"
#include "Connection.hpp"

enum SocketType {
    LISTEN_SOCKET,
    CLIENT_SOCKET
};

struct SocketInfo {
    SocketType type;
    union {
        VirtualServerKey* listen_key; //listen sockets
        Connection* connection; // client sockets  
    } data;
    
    SocketInfo(VirtualServerKey* key) : type(LISTEN_SOCKET) {
        data.listen_key = key;
    }
    
    SocketInfo(Connection* conn) : type(CLIENT_SOCKET) {
        data.connection = conn;
    }
};

#endif