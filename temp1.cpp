#include "../inc/VirtualServersManager.hpp"
#include "../inc/SocketInfo.hpp"
#include "../inc/Connection.hpp"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <vector>
#include <string>
#include <ctime>
#include <unistd.h>

#define MAX_EVENTS 64
#define TIMEOUT_SECONDS 5  // 5 segundos para pruebas
#define EVENTS_TIMEOUT_MILISECONDS 100
#define BUFFER_SIZE 8192

// Configuración de señales para integrar con epoll

// Manejar señales recibidas
static bool handle_signal(int signal_fd, int& server_running) {
	struct signalfd_siginfo signal_info;
	ssize_t bytes_read = read(signal_fd, &signal_info, sizeof(signal_info));
	
	if (bytes_read != sizeof(signal_info)) {
		ERRORlogsEntry("ERROR: ", "Failed to read signal info");
		return false;
	}
	
	switch (signal_info.ssi_signo) {
		case SIGINT:
			OKlogsEntry("SIGNAL: ", "Received SIGINT (Ctrl+C) - Graceful shutdown");
			server_running = 0;
			break;
			
		case SIGTERM:
			OKlogsEntry("SIGNAL: ", "Received SIGTERM - Graceful shutdown");
			server_running = 0;
			break;
			
		case SIGPIPE:
			OKlogsEntry("SIGNAL: ", "Received SIGPIPE - Client disconnected unexpectedly");
			// SIGPIPE se maneja automáticamente, solo loggeamos
			break;
			
		case SIGCHLD:
			OKlogsEntry("SIGNAL: ", "Received SIGCHLD - Child process terminated");
			// Para futuros procesos CGI
			break;
			
		default:
			std::ostringstream oss;
			oss << "Received unknown signal: " << signal_info.ssi_signo;
			OKlogsEntry("SIGNAL: ", oss.str());
			break;
	}
	
	return true;
}

void check_timeouts(std::vector<Connection*>& conn, int epoll_fd) {
	time_t current_time = time(NULL);
	std::vector<Connection*>::iterator it = conn.begin();
	while (it != conn.end()) {
		Connection* current_conn = *it;
		
		// Debug: mostrar tiempo transcurrido
		time_t elapsed = current_time - current_conn->getLastActivity();
		if (elapsed > 2) {  // Solo mostrar si han pasado más de 2 segundos
			std::ostringstream debug_oss;
			debug_oss << "Connection " << current_conn->getSocketFd() << " idle for " << elapsed << " seconds";
			OKlogsEntry("DEBUG: ", debug_oss.str());
		}
		
		if (current_conn->isTimedOut(current_time, TIMEOUT_SECONDS)) {
			std::ostringstream debug_oss;
			debug_oss << "TIMEOUT: Closing connection on socket " << current_conn->getSocketFd();
			OKlogsEntry("DEBUG: ", debug_oss.str());
			
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

	std::vector<SocketInfo*> sockets_infos;
	std::vector<Connection*> connections;
	struct epoll_event events[MAX_EVENTS];

	VirtualServersManager::ServersToSocketsMap::iterator it = sm.getServersToSockets().begin();
	for (; it != sm.getServersToSockets().end(); ++it) {
		sockets_infos.push_back(new SocketInfo(it->first));
		
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.ptr = sockets_infos.back();
		
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
			
			// Debug: imprimir el tipo de socket
			std::ostringstream debug_oss;
			debug_oss << "Event " << i << ": socket_info->type = " << socket_info->type;
			OKlogsEntry("DEBUG: ", debug_oss.str());
			
			// es un cliente nuevo
			if (socket_info->type == LISTEN_SOCKET) {
				int listen_fd = sm.getServersToSockets()[socket_info->listen_key];
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
				Servers::VirtualServerInfo* server = sm.getServerForKey(socket_info->listen_key);
				new_conn->setServer(server);
				
				connections.push_back(new_conn);
				sockets_infos.push_back(new SocketInfo(new_conn));

				struct epoll_event client_event;
				client_event.events = EPOLLIN;
				client_event.data.ptr = sockets_infos.back();

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
				
				// Verificar si el evento es de lectura o escritura
				if (events[i].events & EPOLLIN) {
					// Datos disponibles para leer
					uint8_t buffer[BUFFER_SIZE];
					memset(&buffer, 0, BUFFER_SIZE);
					ssize_t bytes_read = recv(conn->getSocketFd(), buffer, sizeof(buffer) -1, 0);
					
					std::ostringstream debug_oss;
					debug_oss << "recv() on socket " << conn->getSocketFd() << " returned " << bytes_read << " bytes";
					OKlogsEntry("DEBUG: ", debug_oss.str());
					
					if (bytes_read > 0) {
						buffer[bytes_read] = '\0';
						std::string data_received((char*)buffer);
						
						std::ostringstream data_debug;
						data_debug << "Data received (" << bytes_read << " bytes): " << data_received.substr(0, 50) << "...";
						OKlogsEntry("DEBUG: ", data_debug.str());
						
						conn->appendToReadBuffer(data_received);
						conn->updateActivity();
						
						// Procesar estados hasta llegar a uno estable
						ConnectionState prev_state;
						int max_transitions = 5; // Límite de seguridad
						int transitions = 0;
						do {
							prev_state = conn->getState();
							conn->updateConnectionState();
							transitions++;
						} while (prev_state != conn->getState() && transitions < max_transitions);
						
						if (transitions >= max_transitions) {
							std::ostringstream debug_oss;
							debug_oss << "WARNING: Too many state transitions on socket " << conn->getSocketFd();
							OKlogsEntry("DEBUG: ", debug_oss.str());
						}
						
						// Si tenemos respuesta lista, cambiar a modo escritura
						if (conn->getState() == CONN_WRITING_RESPONSE && !conn->getWriteBuffer().empty()) {
							struct epoll_event modify_event;
							modify_event.events = EPOLLOUT;
							modify_event.data.ptr = socket_info;
							epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn->getSocketFd(), &modify_event);
						}
					} else if (bytes_read == 0) {
						// Cliente cerró la conexión - limpiar inmediatamente
						std::ostringstream debug_oss;
						debug_oss << "Client closed connection on socket " << conn->getSocketFd();
						OKlogsEntry("DEBUG: ", debug_oss.str());
						
						// Remover de epoll y limpiar
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->getSocketFd(), NULL);
						conn->closeConnection();
						
						// Encontrar y eliminar de los vectores
						for (std::vector<Connection*>::iterator conn_it = connections.begin(); conn_it != connections.end(); ++conn_it) {
							if (*conn_it == conn) {
								connections.erase(conn_it);
								break;
							}
						}
						continue; // Saltar al siguiente evento
					}
				}
				
				if (events[i].events & EPOLLOUT) {
					// Socket listo para escribir
					if (conn->getState() == CONN_WRITING_RESPONSE && !conn->getWriteBuffer().empty()) {
						const std::string& response = conn->getWriteBuffer();
						ssize_t bytes_sent = send(conn->getSocketFd(), response.c_str(), response.length(), 0);
						if (bytes_sent > 0) {
							conn->updateBytesSent(bytes_sent);
							
							if (conn->responseSent()) {
								std::ostringstream debug_oss;
								debug_oss << "Response sent completely on socket " << conn->getSocketFd();
								OKlogsEntry("DEBUG: ", debug_oss.str());
								
								// Cambiar de vuelta a modo lectura para keep-alive
								struct epoll_event modify_event;
								modify_event.events = EPOLLIN;
								modify_event.data.ptr = socket_info;
								epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn->getSocketFd(), &modify_event);
								
								conn->setState(CONN_KEEP_ALIVE);
							}
						}
					}
				}
			} else {
				throw std::runtime_error("Impossible error.");
			}
		}
		check_timeouts(connections, epoll_fd);
	}
	
	// Limpiar memoria de SocketInfo
	for (size_t i = 0; i < sockets_infos.size(); ++i) {
		delete sockets_infos[i];
	}
	
	return true;
}