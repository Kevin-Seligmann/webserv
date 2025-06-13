#include "../inc/Connection.hpp"
#include "../inc/Utils.hpp"
#include <unistd.h>
#include <iostream>

Connection::Connection(int fd)
	: _socket_fd(fd)
	, _last_activity(time(NULL))
	, _state(CONN_IDLE)
	, _read_buff("")
	, _write_buff("")
	{}

Connection::~Connection() {
	closeConnection();
};

int Connection::getSocketFd() const { return _socket_fd; }
void Connection::setSocketFd(int fd) { _socket_fd = fd; }

time_t Connection::getLastActivity() const { return _last_activity; }
void Connection::setLastActivity(time_t last_activity) { _last_activity = last_activity; }

ConnectionState Connection::getState() const { return _state; }
void Connection::setState(ConnectionState state) { 
	_state = state;
	updateActivity();
}

const std::string& Connection::getReadBuffer() const { return _read_buff; }
void Connection::appendToReadBuffer(const std::string& data) {
	_read_buff += data;
	updateActivity();
}

const std::string& Connection::getWriteBuffer() const { return _write_buff; }
void Connection::setWriteBuffer(const std::string& data) {
	_write_buff = data; 
	updateActivity();
}

void Connection::clearBuffers() {
	_read_buff.clear();
	_write_buff.clear();
};

void Connection::updateActivity() {
	_last_activity = time(NULL);
};

static bool headerComplete(const std::string& read_buffer) {
	return true;
	// buscar que acabe en \r\n\r\n
}

static bool hasBody(const std::string& read_buffer) {
	return true;
	// buscar la key body_size (?)
	// es posible que el argumento que reciba se el objeto HttpReques parseado
}

static bool bodyComplete(const std::string& read_buffer) {
	return true;
	// comparar size de body con key body_size de los headers
}

static bool responseSent(const std::string& write_buffer) {
	// comprobar si la response está ready
	return true;
}

void Connection::updateConnectionState() {
	switch (getState())	{

		case CONN_IDLE:
			// Estado inicial - solo cambiar cuando lleguen datos
			if (!getReadBuffer().empty()) {
				OKlogsEntry("CONNECTION: ", "Receiving data from socket: " + getSocketFd());
				setState(CONN_READING_HEADER);
			}
			break;

		case CONN_READING_HEADER:
			if (headerComplete(getReadBuffer())) {
				if (hasBody(getReadBuffer())) {
					setState(CONN_READING_BODY);
				} else {
					// processRequest() de Kevin
				}				
			}
			break;

		case CONN_READING_BODY:
			if (bodyComplete(getReadBuffer())) {
				OKlogsEntry("CONNECTION: ", "Complete request from socket: " + getSocketFd());
				setState(CONN_PROCESSING)
			;}
			break;

		case CONN_PROCESSING:
			if (getWriteBuffer().empty()) {
				// processRequest() de Kevin
				// fake response por ahora
				setWriteBuffer("HTTP/1.1 200 OK\r\n\r\nHello World!");
			}
			if (!getWriteBuffer().empty()) {
				setState(CONN_WRITING_RESPONSE);
			}
			break;

		case CONN_WRITING_RESPONSE:
			if (responseSent(getWriteBuffer()));
			// TODO: send() response to client
			// Por ahora, simular respuesta enviada
			setState(CONN_IDLE);  // O CONN_CLOSING si no keep-alive
			break;

		case CONN_KEEP_ALIVE:
			if (!getReadBuffer().empty()) {
				setState(CONN_READING_HEADER);
			}

		default:
			break;
		}
}

bool Connection::isTimedOut(time_t current_time, int timeout_seconds) const {
	return ((current_time - _last_activity) > timeout_seconds);
}

void Connection::closeConnection() {
	if (_socket_fd >= 0) {
		::close(_socket_fd);
		_socket_fd = -1;
		_state = CONN_CLOSED;
	}
};