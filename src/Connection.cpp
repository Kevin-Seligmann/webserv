#include "../inc/Connection.hpp"
#include "../inc/Server.hpp"
#include "../inc/Utils.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>

Connection::Connection(int fd)
	: _socket_fd(fd)
	, _last_activity(time(NULL))
	, _state(CONN_READING_REQUEST)
	, _response_complete(false)
	, _bytes_to_send(0)
	, _bytes_sent(0)
	, _server(NULL)
	, _read_buff("")
	, _write_buff("")
//	, _response(NULL)
	, _cgi_read_fd(0)
	, _cgi_process_pid(0)
	{}

Connection::~Connection() {
	closeConnection();
};

// getters/setters en el mismo orden que los miembros privados
int Connection::getSocketFd() const { return _socket_fd; }
void Connection::setSocketFd(int fd) { _socket_fd = fd; }

time_t Connection::getLastActivity() const { return _last_activity; }
void Connection::setLastActivity(time_t last_activity) { _last_activity = last_activity; }

ConnectionState Connection::getState() const { return _state; }
void Connection::setState(ConnectionState state) {
	_state = state;
	std::string din_state;
	switch (state) {
		case CONN_READING_REQUEST:
			din_state = "CONN_READING_REQUEST";
			break;
		case CONN_PROCESSING_REQUEST:
			din_state = "CONN_PROCESSING_REQUEST";
			break;
		case CONN_WRITING_RESPONSE:
			din_state = "CONN_WRITING_RESPONSE";
			break;
		case CONN_KEEP_ALIVE:
			din_state = "CONN_KEEP_ALIVE";
			break;
		case CONN_CLOSED:
			din_state = "CONN_CLOSED";
			break;
		default:
			break;
	}
	OKlogsEntry("STATUS: ", "switching to " + din_state);
	updateActivity();
}

bool Connection::getResponseComplete() const { return _response_complete; }
void Connection::setResponseComplete(bool response_complete) { _response_complete = response_complete; }

size_t Connection::getBytesToSend() const { return _bytes_to_send; }
void Connection::setBytesToSend(size_t bytes_to_send) { _bytes_to_send = bytes_to_send; }

size_t Connection::getBytesSent() const { return _bytes_sent; }
void Connection::setBytesSent(size_t bytes_sent) { _bytes_sent = bytes_sent; }

Server* Connection::getServer() const { return _server; }
void Connection::setServer(Server* server) { _server = server; }

const std::string& Connection::getReadBuffer() const { return _read_buff; }
void Connection::appendToReadBuffer(const std::string& data) { _read_buff += data; updateActivity(); }

const std::string& Connection::getWriteBuffer() const { return _write_buff; }
void Connection::setWriteBuffer(const std::string& data) { _write_buff = data; updateActivity(); }
void Connection::appendToWriteBuffer(const std::string& data) { _write_buff += data; updateActivity(); }

// HttpResponse* Connection::getHttpResponse() const { return _response; }
// void Connection::setHttpResponse(HttpResponse resp) { _response = &resp; }

// manejo de conexion
void Connection::clearBuffers() {
	_read_buff.clear();
	_write_buff.clear();
};

void Connection::updateActivity() {
	_last_activity = time(NULL);
};

// Métodos para el bucle de eventos
bool Connection::isRequestComplete() const {
	// Verificar si tenemos un request HTTP completo
	// Buscar \r\n\r\n para headers completos
	size_t header_end = _read_buff.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		return false; // Headers incompletos
	}
	
	// TODO: Verificar Content-Length para body si es necesario
	// Por ahora asumimos que headers completos = request completo
	return true;
}

bool Connection::hasResponseReady() const {
	return !_write_buff.empty();
}

void Connection::processRequest() {
	if (!isRequestComplete()) {
		return;
	}
	
	setState(CONN_PROCESSING_REQUEST);
	
	// TODO: Aquí iría el procesamiento real del request
	// Por ahora generar una respuesta simple
	std::string response = "HTTP/1.1 200 OK\r\n"
						  "Content-Type: text/html\r\n"
						  "Content-Length: 13\r\n"
						  "Connection: keep-alive\r\n"
						  "\r\n"
						  "Hello, World!";
	
	setWriteBuffer(response);
	_bytes_to_send = response.length();
	_bytes_sent = 0;
	setState(CONN_WRITING_RESPONSE);
}

void Connection::resetForNextRequest() {
	_read_buff.clear();
	_write_buff.clear();
	_bytes_to_send = 0;
	_bytes_sent = 0;
	setState(CONN_READING_REQUEST);
}

bool Connection::responseSent() const {
	return _bytes_sent >= _bytes_to_send && _bytes_to_send > 0;
}

void Connection::updateBytesSent(size_t sent_bytes) {
	_bytes_sent += sent_bytes;
	updateActivity();
}

bool Connection::isTimedOut(time_t current_time, int timeout_seconds) const {
	return ((current_time - _last_activity) > timeout_seconds);
}

void Connection::closeConnection() {
	if (_socket_fd >= 0) {
		::close(_socket_fd);
		_socket_fd = -1;
		setState(CONN_CLOSED);
	}
};