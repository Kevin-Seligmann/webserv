#include "../inc/VirtualServersManager.hpp"
#include "../inc/SocketInfo.hpp"
#include "../inc/Connection.hpp"
#include <sys/epoll.h>
#include <vector>
#include <string>
#include <ctime>
#include <unistd.h>

#define MAX_EVENTS 64
#define TIMEOUT_MILISECONDS 10000
#define EVENTS_TIMEOUT_MILISECONDS 100
#define BUFFER_SIZE 8192

void check_timeouts(std::vector<Connection*>& conn, int epoll_fd) {
	std::vector<Connection*>::iterator it = conn.begin();
	while (it != conn.end()) {
		Connection* current_conn = *it;
		if (current_conn->isTimedOut(time(NULL), TIMEOUT_MILISECONDS)) {
			current_conn->closeConnection();
			current_conn->clearBuffers();
			epoll_ctl(epoll_fd,  EPOLL_CTL_DEL, current_conn->getSocketFd(), NULL);
			it = conn.erase(it);
		} else {
			++it;
		} 
	}
}

bool event_loop(VirtualServersManager& sm) {
	int epoll_fd = epoll_create(1);
	if (epoll_fd == -1) {
		throw std::runtime_error("Couldn't create epoll.");
	}

	std::vector<SocketInfo> sockets_infos;
	std::vector<Connection*> connections;
	struct epoll_event events[MAX_EVENTS];

	VirtualServersManager::ServersToSocketsMap::iterator it = sm.serversToSockets.begin();
	for (; it != sm.serversToSockets.end(); ++it) {
		sockets_infos.push_back(SocketInfo(it->first));
		
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.ptr = &sockets_infos.back();
		
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, it->second, &event) == -1) {
			ERRORlogsEntry("ERROR: ", "Couldn't add listen socket to epoll.");
			throw std::runtime_error("Couldn't add listen socket to epoll.");
		}
	}
	
	int server_running = 1;
	while (server_running) {
		int count_events = epoll_wait(epoll_fd, events, MAX_EVENTS, EVENTS_TIMEOUT_MILISECONDS);

		for (int i = 0; i < count_events; ++i) {
			SocketInfo* socket_info = (SocketInfo*)events[i].data.ptr;
			// es un cliente nuevo
			if (socket_info->type == LISTEN_SOCKET) {
				int listen_fd = sm.serversToSockets[socket_info->listen_key];
				struct sockaddr_in client_addr;
					// client_addr.sin_addr.s_addr -> IP
					// client_addr.sin_port -> PORT
				socklen_t client_len = sizeof(client_addr); 
				
				int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
				if (client_fd == -1) {
					ERRORlogsEntry("ERROR: ", "Couldn't accept new connection.");
					continue;
				}
				
				Connection* new_conn = new Connection(client_fd);
				connections.push_back(new_conn);
				sockets_infos.push_back(SocketInfo(new_conn));

				struct epoll_event client_event;
				client_event.events = EPOLLIN;
				client_event.data.ptr = &sockets_infos.back();

				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
					ERRORlogsEntry("ERROR: ", "Couldn't add client_fd to epoll.");
					delete new_conn;
					connections.pop_back();
					sockets_infos.pop_back();
					close(client_fd);
					continue;
				}
			// es un cliente activo
			} else if (socket_info->type == CLIENT_SOCKET) {
				Connection* conn = socket_info->connection;
				uint8_t buffer[BUFFER_SIZE];
				memset(&buffer, 0, BUFFER_SIZE);
				ssize_t bytes_read = recv(conn->getSocketFd(), buffer, sizeof(buffer) -1, 0);
				if (bytes_read > 0) {
					buffer[bytes_read] = '\0'; // es redundante con memset pero es prolijo, creo
					conn->appendToReadBuffer(std::string((char*)buffer));
					conn->updateActivity();
					conn->updateConnectionState();
				}
			} else {
				throw std::runtime_error("Impossible error.");
			}
		}
		check_timeouts(connections, epoll_fd);
	}
}