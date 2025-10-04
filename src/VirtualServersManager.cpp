#include "VirtualServersManager.hpp"
#include <cstring>  // memset
#include <unistd.h>  // close
#include <arpa/inet.h>  // inet_pton
#include <errno.h>  // inet_pton

// ================ SIGNALS  MANAGEMENT ================
static void make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) flags = 0;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set O_NONBLOCK");
    }
}

volatile sig_atomic_t VirtualServersManager::s_shutdown_requested = 0;

void VirtualServersManager::setupSignals() {

	// SIGPIPE manage signals launched at pipes closure (we want to make suer it is ignored)
	struct sigaction sa_pipe;
	sa_pipe.sa_handler = SIG_IGN;
	// Set signals to block during execution of handler
	sigemptyset(&sa_pipe.sa_mask);
	// Set behaviour for process interrupted by handler
	sa_pipe.sa_flags = 0;
	

	// SIGINT/SIGTERM launched buy user o kill, initiates gentle shutdown
	struct sigaction sa_shutdown;
	sa_shutdown.sa_handler = signal_handler;
	// Set signals to block during execution of handler
	sigemptyset(&sa_shutdown.sa_mask);
	// Block this signals during handler execution to avoid reentrance
	sigaddset(&sa_shutdown.sa_mask, SIGINT);
	sigaddset(&sa_shutdown.sa_mask, SIGTERM);
	// Set behaviour for process interrupetd by handler
	sa_shutdown.sa_flags = SA_RESTART;

/*
	// SIGCHLD manage signlas launched at child termination, avoid zombie processes
	struct sigaction sa_child;
	sa_child.sa_handler = sigchild_handler;
	// Set signals to block during execution of handler
	sigemptyset(&sa_child.sa_mask);
	// Block other signals during handler execution (reaping)
	sigaddset(&sa_child.sa_mask, SIGINT);
	sigaddset(&sa_child.sa_mask, SIGTERM);
	// Set behaviour for process interrupted by handler
	sa_child.sa_flags = SA_RESTART | SA_NOCLDSTOP;
*/

	// Apply config to signals
	
	// SIGPIPE
	if (sigaction(SIGPIPE, &sa_pipe, NULL ) == -1) 
	{
		throw std::runtime_error("Failed to install SIGPIPE handler");
	}

	// SIGINT
	if (sigaction(SIGINT, &sa_shutdown, NULL ) == -1) 
	{
		throw std::runtime_error("Failed to install SIGINT handler");
	}

	// SIGTERM
	if (sigaction(SIGTERM, &sa_shutdown, NULL) == -1) 
	{
		throw std::runtime_error("Failed to install SIGTERM handler");
	}

	/*
	// SIGCHLD
	if (sigaction(SIGCHLD, &sa_child, NULL ) == -1) 
	{
		throw std::runtime_error("Failed to install SIGCHLD handler");
	}
	*/	
}

void VirtualServersManager::signal_handler(int sig) {
	if (sig == SIGINT || sig == SIGTERM) {
		s_shutdown_requested = 1;	
	}
}

void VirtualServersManager::sigchild_handler(int sig) {
	(void)sig;

	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		// Nothing here
		// Waitpid removes de process from the porocess table
		// It only iterates to clean every child finished
		// If need I can implement a self pipe trick to add actions in the run loop
	}
}

// ================ CONSTRUCTORS & DESTRUCTOR ================

VirtualServersManager::VirtualServersManager(){
}

VirtualServersManager::VirtualServersManager(const ParsedServers& configs){

	std::vector<ParsedServer>::const_iterator it = configs.begin();
	for (;it!= configs.end(); ++it) {
		_server_configs.push_back(ServerConfig(*it));
	}

	for (size_t i = 0; i < _server_configs.size(); ++i) {
		ServerConfig* cfg = &_server_configs[i];
		const ParsedServer& parsed = configs[i];

		std::vector<Listen>::const_iterator l_it = parsed.listens.begin();
		for (; l_it != parsed.listens.end(); ++l_it) {
			const Listen listen = *l_it;
			// Mapeo de listen y vector de configuraciones
			_virtual_hosts[listen].push_back(cfg);

			// Par único listen - socket (será creado)
			if (_listen_sockets.find(listen) == _listen_sockets.end()) {
				_listen_sockets[listen] = -1; // Asignar socket en POLLING
			} 
		}
	}
	Logger::getInstance().info("VirtualServersManager: Loaded " + wss::i_to_dec(_server_configs.size()) + " configs");
    Logger::getInstance().info("VirtualServersManager: Created " + wss::i_to_dec(_listen_sockets.size()) + " listen ports");
}

VirtualServersManager::~VirtualServersManager() {
	// Sockets
	std::map<Listen, int>::iterator sock_it = _listen_sockets.begin();
	for (;sock_it != _listen_sockets.end(); ++sock_it) {
		if (sock_it->second >= 0) {
			close(sock_it->second);
		}
	}

	// Clients
	std::map<int, Client*>::iterator cli_it = _clients.begin();
	for (; cli_it != _clients.end(); ++cli_it) {
		delete cli_it->second;
	}
	_clients.clear();

	Logger::getInstance().info("VirtualServersManager destroyed");
}

// ================ SOCKETS / POLL =================

void VirtualServersManager::setPolling() {
    Logger::getInstance().info("Starting polling configuration (setPolling)");
    
    Logger::getInstance() << "Creating sockets for " << _server_configs.size() << " servers" << std::endl;
    
	for (std::map<Listen, int>::iterator it = _listen_sockets.begin();
		it != _listen_sockets.end(); ++it) {

		it->second = createAndBindSocket(it->first);
		Logger::getInstance() << "Server " << it->first.host << ":" << it->first.port 
							  << " started on socket FD: " << it->second;

		_wspoll.add(it->second, POLLIN);
		Logger::getInstance().info("Socket created and added to poll");
    }
	Logger::getInstance().info("Setup of poll complete");
}	

int VirtualServersManager::createAndBindSocket(const Listen& listen_arg) {
	int socket_fd = socket (AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		throw std::runtime_error("Faile to creat socket");
	}
	make_socket_nonblocking(socket_fd);

	int opt = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listen_arg.port);

	if (listen_arg.host == "0.0.0.0" || listen_arg.host.empty()) {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		inet_pton(AF_INET, listen_arg.host.c_str(), &addr.sin_addr);
	}

	if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(socket_fd);
		throw std::runtime_error("Bind failed");
	}

	if (listen(socket_fd, 128) < 0) {
		close (socket_fd);
		throw std::runtime_error("Listen failed");
	}
	return socket_fd;
}

bool VirtualServersManager::isListenSocket(int socket_fd) const {
	for (std::map<Listen, int>::const_iterator it = _listen_sockets.begin();
		it != _listen_sockets.end(); ++it) {
		if (it->second == socket_fd) {
			return true;
		}
	}
	return false;
}

// ================ CLIENT ================

Client* VirtualServersManager::searchClient(int client_fd) {
	std::map<int, Client *>::iterator it = _clients.find(client_fd);
	if (it != _clients.end()) {
		return it->second;
	}

	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it ++)
		if (it->second->ownsFd(client_fd))
			return it->second;
	return NULL;
}

void VirtualServersManager::disconnectClient(int client_fd) {
	std::cout << "Disconnecting client FD: " << client_fd << std::endl;
	
	_client_to_listen.erase(client_fd);

	std::map<int, Client*>::iterator it = _clients.find(client_fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
		return ;
	}

	CODE_ERR("The server tried to find a client that does not exists. This is not possible. " + wss::i_to_dec(client_fd)); // Maybe it's possible
}

// ================ EVENT ================

void VirtualServersManager::handleEvent(const struct Wspoll_event event) {
	int socket_fd = event.fd;

	if (event.events & POLLIN && isListenSocket(socket_fd)) 
	{
		handleNewConnection(socket_fd);
	} else {
		try {
			Client* client = searchClient(socket_fd);
			if (!client)
			{
				// unhookFileDescriptor(ActiveFileDescriptor(socket_fd, 0));
				// CODE_ERR("A file descriptor that doesn't belong to any client has been found: " + wss::i_to_dec(socket_fd));
				Logger::getInstance() << "A file descriptor without client has been found: " << socket_fd << std::endl; 
				_wspoll.del(socket_fd);
				return ;
			}

			if (event.events & POLLERR)
			{
				Logger::getInstance() << "Client " + wss::i_to_dec(client->getSocket())+ " POLLERR on socket " << socket_fd << ": "<< strerror(errno) << std::endl;
				disconnectClient(client->getSocket());
			}
			else if (event.events & POLLRDHUP && client->idle())
			{
				Logger::getInstance() << "Client " + wss::i_to_dec(client->getSocket())+ ": connection closed by peer " << std::endl;
				disconnectClient(client->getSocket());
			}
			else 
			{
				client->process(socket_fd, event.events);
			}

		} catch (const std::runtime_error& e) {
			Logger::getInstance() << "Exception processing client data: " << e.what() << std::endl;
			DEBUG_LOG("Exception processing client data: " << e.what());
			disconnectClient(socket_fd);
		}
	}
}

void VirtualServersManager::handleNewConnection(int listen_fd) {
	Listen* listen = NULL;
	for (std::map<Listen, int>::iterator it = _listen_sockets.begin();
		it != _listen_sockets.end(); ++it) {
			if (it->second == listen_fd) {
				listen = const_cast<Listen*>(&it->first);
				break;
			}
	}
	
	if (!listen) {
		return;
	}

	struct sockaddr_in client_addr;
	std::memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		return;
	}
	make_socket_nonblocking(client_fd);

	if (_wspoll.is_full())
	{
		close(client_fd);
		return ;
	}

	_client_to_listen[client_fd] = *listen;
	
	_clients[client_fd] = new Client(*this, client_fd);
}

// ================ POLL FD MANAGEMENT ================

void VirtualServersManager::hookFileDescriptor(ActiveFileDescriptor const & actf)
{
	_wspoll.add(actf.fd, actf.mode);
}

void VirtualServersManager::unhookFileDescriptor(ActiveFileDescriptor const & actf)
{
	_wspoll.del(actf.fd);
}

void VirtualServersManager::updateFiledescriptor(ActiveFileDescriptor const & actf)
{
	_wspoll.mod(actf.fd, actf.mode);
}

// ================ REQUEST ROUTER ================

ServerConfig* VirtualServersManager::findServerConfigForRequest(const HTTPRequest& request, int client_fd) {
	std::map<int, Listen>::iterator it = _client_to_listen.find(client_fd);
	if (it == _client_to_listen.end())
		CODE_ERR("Impossible error trying to find server for client " + wss::i_to_dec(client_fd));

	std::map<Listen, std::vector<ServerConfig*> >::iterator vh_it = _virtual_hosts.find(it->second);
	if (vh_it == _virtual_hosts.end())
		CODE_ERR("Impossible error trying to find server for client " + wss::i_to_dec(client_fd));

	if (vh_it->second.empty())
		CODE_ERR("No server configurations found for " + it->second.host + ":" + wss::i_to_dec(it->second.port));

	std::string hostname = request.get_host();
	for (size_t i = 0; i < vh_it->second.size(); ++i)
	{
		if (!vh_it->second[i])
		{
			continue;
		}

		if (vh_it->second[i]->matchesServerName(hostname))
		{
			return vh_it->second[i];
		}
	}

	if (!vh_it->second[0])
		CODE_ERR("Default server configuration not found for " + it->second.host + ":" + wss::i_to_dec(it->second.port));

	return vh_it->second[0]; // First one is default
}

// ================ MAIN LOOP ================

void VirtualServersManager::run() {
	Logger::getInstance().info("=========== Starting WEBSERVER ===========");

	try {
		setupSignals();
		s_shutdown_requested = 0;
		_loop_counter = 0;
		setPolling();
	} catch (const std::exception& e) {
		Logger::getInstance().error("Setup falied: " + std::string(e.what()));
		return;
	}

	Logger::getInstance().info("=========== WEBSERVER Started. Listening to new requests ===========");

	while (!s_shutdown_requested) 
	{
		int incoming = _wspoll.wait();

		if (incoming < 0)
		{
			if (errno == EINTR && s_shutdown_requested)
				break;
			else
				throw std::runtime_error("Fatal error: Poll failed");
		}

		_loop_counter ++;
		if (_loop_counter % 1000000 == 0)
			{
				Logger::getInstance() << "Running ... " << std::endl;
				_loop_counter = 0;
			}
					
		int active_event = 0;
		for (int i = 0; i < _wspoll.size(); ++i) {
			if (_wspoll[i].events)
			{
				try
				{
					active_event ++;
					handleEvent(_wspoll[i]);

				} 
				catch (const std::exception& e)
				{
					Logger::getInstance().error("Critial error handling event: " + std::string(e.what()) + " The server must close. ");
					throw std::exception();
				}
			}
		}
		if (_loop_counter == 0)
			Logger::getInstance() << "Active events: " << active_event <<std::endl;

		checkTimeouts();
		killZombies();
	}
	gracefulShutdown();
}

void VirtualServersManager::killZombies()
{
	while (1)
	{
		int status;
		int res = waitpid(-1, &status, WNOHANG);
		if (res > 0)
			continue ;
		break ;
	}
}

void VirtualServersManager::checkTimeouts() {
	time_t now = time(NULL);
	std::vector<int> to_disconnect;
	std::vector<int> to_close;

	std::map<int, Client*>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it)
	{
		time_t inactive_time = now - it->second->getLastActivity();

		if (it->second->closing())
		{
			if (inactive_time > Client::CLOSING_GRACE_PERIOD)
			{
				to_close.push_back(it->first);
			}
		}
		else
		{
			if (inactive_time > Client::TIMEOUT_SECONDS)
			{
				to_disconnect.push_back(it->first);
			}
		}
	}

	for (size_t i = 0; i < to_disconnect.size(); ++i)
	{
		Logger::getInstance().warning("Client " + wss::i_to_dec(to_disconnect[i]) + " timeout");
		disconnectClient(to_disconnect[i]);
	}

	for (size_t i = 0; i < to_close.size(); ++i) {
		disconnectClient(to_close[i]);
	}
}

void VirtualServersManager::gracefulShutdown() {
	Logger::getInstance().info("Shutdown signal received, closing server gracefully...");

	std::map<Listen, int>::iterator it = _listen_sockets.begin();
	for (; it != _listen_sockets.end(); ++it) {
		if (it->second >= 0) {
			close(it->second);
		}
	}

	time_t start = time(NULL);
	while (!_clients.empty()) {
		time_t waiting = time(NULL) - start;
		if (waiting > Client::KEEP_ALIVE_TIMEOUT) {
			Logger::getInstance().warning("Timeout. Shuting down.");
			break;
		}

		_wspoll.wait();

	
		for (int i = 0; i < _wspoll.size(); ++i) {
			if (_wspoll[i].events) {
				try {
					handleEvent(_wspoll[i]);
				} catch (...) { 
				}
			}

		}

		checkTimeouts();
	}

	Logger::getInstance().info("=========== Closing EVENT LOOP ===========");
}
