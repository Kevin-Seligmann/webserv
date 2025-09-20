#ifndef VIRTUAL_SERVERS_MANAGER_HPP
#define VIRTUAL_SERVERS_MANAGER_HPP

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <cstring>
#include "Wspoll.hpp"
#include "ServerConfig.hpp"
#include "HTTPRequest.hpp"
#include "RequestManager.hpp"
#include "HTTPError.hpp"
#include "ElementParser.hpp"
#include "SysBufferFactory.hpp"
#include "ResponseManager.hpp"
#include "Location.hpp"
#include "HTTPMethod.hpp"
#include "Status.hpp"
#include "Listen.hpp"
#include "Client.hpp"

class VirtualServersManager {

private:
	std::vector<ServerConfig>						_server_configs; // Todas las configuraciones
	std::map<Listen, std::vector<ServerConfig*> >	_virtual_hosts; // Listen, configs
	std::map<Listen, int>							_listen_sockets; // Listen, socket_fd
	std::map<int, Listen>							_client_to_listen; // client_fd to Listen, para llegar al vhost
	std::map<int, Client*>                          _clients;

	Wspoll                                          _wspoll;
 
	// Manejo de señales
	static VirtualServersManager*                   s_instance;
	static volatile sig_atomic_t					s_shutdown_requested;

	// Métodos privados de socket management
	int createAndBindSocket(const Listen& listen);
	bool isListenSocket(int fd) const;
	void handleEvent(const struct Wspoll_event event);
	void handleNewConnection(int listen_fd);
	void setPolling();
	
	Client* searchClient(int client_fd);
	
	public:
	VirtualServersManager();
	VirtualServersManager(const ParsedServers& configs);
	~VirtualServersManager();
	
	void run();
	
	void hookFileDescriptor(const ActiveFileDescriptor& actf);
	void unhookFileDescriptor(const ActiveFileDescriptor& actf);
	void swapFileDescriptor(const ActiveFileDescriptor& oldfd, const ActiveFileDescriptor& newfd);
	void disconnectClient(int client_fd);

	ServerConfig* findServerConfigForRequest(const HTTPRequest& request, int client_fd);
	void checkTimeouts();

	// Manejo de señales
	static void signal_handler(int sig);
	void setupSignals();
	void gracefulShutdown();
	bool isShutdownRequested() const { return s_shutdown_requested != 0; }
};
#endif

/*
                        PARSEO (ParsedServers)
                            |
                            | Constructor VSM
                            v
    ┌──────────────────────────────────────────────────────────┐
    │               VirtualServersManager                      │
    │                                                          │
    │  ┌──────────────────────────────────────────────┐        │
    │  │ _server_configs: vector<ServerConfig>        │<───────│─── OWNERSHIP
    │  │ [0] ServerConfig { names, locations, ... }   │        │    (datos completos)
    │  │ [1] ServerConfig { names, locations, ... }   │        │
    │  │ [2] ServerConfig { names, locations, ... }   │        │
    │  └──────────────────────────────────────────────┘        │
    │           │ ^                                            │
    │           │ │ Punteros                                   │
    │           v │                                            │
    │  ┌──────────────────────────────────────────────┐        │
    │  │ _virtual_hosts: map<Listen, vector<SC*>>    │<────────│─── MAPEO LÓGICO
    │  │ {127.0.0.1:80} -> [SC*0, SC*1]              │         │    (referencias)
    │  │ {127.0.0.1:443} -> [SC*2]                   │         │
    │  └──────────────────────────────────────────────┘        │
    │           │                                              │
    │           │ createAndBindSocket()                        │
    │           v                                              │
    │  ┌──────────────────────────────────────────────┐        │
    │  │ _listen_sockets: map<Listen, int>            │<───────│─── SOCKETS FÍSICOS
    │  │ {127.0.0.1:80} -> fd:3                       │        │    (file descriptors)
    │  │ {127.0.0.1:443} -> fd:4                      │        │
    │  └──────────────────────────────────────────────┘        │
    │           │                                              │
    │           │ accept() → client_fd                         │
    │           v                                              │
    │  ┌──────────────────────────────────────────────┐        │
    │  │ _client_to_listen: map<int, Listen>          │<───────│─── TRACKING CLIENTES
    │  │ client_fd:5 -> {127.0.0.1:80}                │        │
    │  │ client_fd:6 -> {127.0.0.1:80}                │        │
    │  └──────────────────────────────────────────────┘        │
    │           │                                              │
    │           │ new Client()                                 │
    │           v                                              │
    │  ┌──────────────────────────────────────────────┐        │
    │  │ _clients: map<int, Client*>                  │<───────│─── OBJETOS CLIENTE
    │  │ fd:5 -> Client* ───┐                         │        │
    │  │ fd:6 -> Client*    │                         │        │
    │  └────────────────────┼─────────────────────────┘        │
    └───────────────────────┼──────────────────────────────────┘
                            │
                            v
    ┌───────────────────────────────────────────────────────┐
    │                    Client                             │
    │  _socket: 5                                           │
    │  _vsm: VirtualServersManager&                         │
    │  _request: HTTPRequest { host: "example.com" }        │
    │                                                       │
    │  get_config(&server, &location)                       │
    │      │                                                │
    │      ├─> vsm.getServerConfig(_socket, _request)       │
    │      │       │                                        │
    │      │       ├─> _client_to_listen[5] → Listen        │
    │      │       ├─> _virtual_hosts[Listen] → [SC*]       │
    │      │       └─> matchServerConfig(vhosts, host)      │
    │      │                                                │
    │      └─> server->findLocation(path)                   │
    └───────────────────────────────────────────────────────┘

    */
