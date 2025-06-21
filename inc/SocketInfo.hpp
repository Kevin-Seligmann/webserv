#ifndef SOCKETINFO_HPP
#define SOCKETINFO_HPP

#include "VirtualServersManager.hpp"
#include "Connection.hpp"

enum SocketType {
	LISTEN_SOCKET,
	CLIENT_SOCKET
};

struct SocketInfo {
	SocketType					type;
	Listen::VirtualServerKey	listen_key;
	Connection*					connection;
	
	SocketInfo(const Listen::VirtualServerKey& key) 
		: type(LISTEN_SOCKET)
		, listen_key(key)
		, connection(NULL) {
	}
	
	SocketInfo(Connection* conn) 
		: type(CLIENT_SOCKET)
		, listen_key("", 0)
		, connection(conn) {
	}
};

#endif