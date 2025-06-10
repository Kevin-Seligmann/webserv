#include "../inc/VirtualServersManager.hpp"


void check_timeouts() {
	time_t now = time(NULL);
	for (connections...) {
		if (now - connection.last_activity > TIMEOUT_SECONDS) close_connection(connection);
	}
}

bool main_loop(VirtualServersManager& serversManager) {
    int epoll_fd = epoll_create(1);
    
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = &socket_info...
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &event);


    int server_running = 1;
    while (server_running) {
        int count_events = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout_ms);

        for (int i = 0; i < count_events; ++i) {
            if (es_de_escuha(events[i])) {
                // accept
            }
            else if (es_de_cliente(events[i])) {
                // recv y procesar
            }
        }
        check_timeouts();
        // cerrar expiradas
    }
}