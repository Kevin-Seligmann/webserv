#include "../inc/Connection.hpp"
#include "../inc/Utils.hpp"
#include "../inc/Servers.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>

Connection::Connection(int fd)
	: _socket_fd(fd)
	, _last_activity(time(NULL))
	, _state(CONN_IDLE)
	, _response_complete(false)
	, _bytes_to_send(0)
	, _bytes_sent(0)
	, _server(NULL)
	, _read_buff("")
	, _request(NULL)
	, _write_buff("")
	, _response(NULL)
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
void Connection::setState(ConnectionState state) { _state = state; updateActivity(); }

bool Connection::getResponseComplete() const { return _response_complete; }
void Connection::setResponseComplete(bool response_complete) { _response_complete = response_complete; }

size_t Connection::getBytesToSend() const { return _bytes_to_send; }
void Connection::setBytesToSend(size_t bytes_to_send) { _bytes_to_send = bytes_to_send; }

size_t Connection::getBytesSent() const { return _bytes_sent; }
void Connection::setBytesSent(size_t bytes_sent) { _bytes_sent = bytes_sent; }

Servers::VirtualServerInfo* Connection::getServer() const { return _server; }
void Connection::setServer(Servers::VirtualServerInfo* server) { _server = server; }

const std::string& Connection::getReadBuffer() const { return _read_buff; }
void Connection::appendToReadBuffer(const std::string& data) { _read_buff += data; updateActivity(); }

HttpRequest* Connection::getHttpRequest() const { return _request; }
void Connection::setHttpRequest(HttpRequest* req) { _request = req; }

const std::string& Connection::getWriteBuffer() const { return _write_buff; }
void Connection::setWriteBuffer(const std::string& data) { _write_buff = data; updateActivity(); }
void Connection::appendToWriteBuffer(const std::string& data) { _write_buff += data; updateActivity(); }

HttpResponse* Connection::getHttpResponse() const { return _response; }
void Connection::setHttpResponse(HttpResponse resp) { _response = &resp; }

// manejo de conexion
void Connection::clearBuffers() {
	_read_buff.clear();
	_write_buff.clear();
};

void Connection::updateActivity() {
	_last_activity = time(NULL);
};

static bool headerComplete(const std::string& read_buffer) {
	return read_buffer.find("\r\n\r\n") != std::string::npos;
}

static bool hasBody(const std::string& read_buffer) {
	// Para esta implementación simple, asumimos que GET requests no tienen body
	// TODO: Implementar parsing real de Content-Length, Transfer-Encoding, etc.
	return read_buffer.find("POST ") == 0 || read_buffer.find("PUT ") == 0;
}

static bool bodyComplete(const std::string& read_buffer) {
	// Para esta implementación simple, si llegamos aquí el body está completo
	// TODO: Implementar verificación real basada en Content-Length
	(void)read_buffer;
	return true;
}

void Connection::updateConnectionState() {
	ConnectionState current_state = getState();
	std::ostringstream debug_oss;
	debug_oss << "updateConnectionState: Current state = " << current_state;
	OKlogsEntry("DEBUG: ", debug_oss.str());
	
	switch (current_state)	{

		case CONN_IDLE: {
			std::ostringstream oss;
			oss << getSocketFd();
			OKlogsEntry("CONNECTION: ", "Receivng data from socket: " + oss.str());
			setState(CONN_READING_HEADER);
			break;
		}

		case CONN_READING_HEADER: {
			OKlogsEntry("DEBUG: ", "In CONN_READING_HEADER state");

			if (headerComplete(getReadBuffer())) {
				OKlogsEntry("DEBUG: ", "Header complete detected");
				if (hasBody(getReadBuffer())) {
					OKlogsEntry("DEBUG: ", "Body detected, going to CONN_READING_BODY");
					setState(CONN_READING_BODY);
				} else {
					OKlogsEntry("DEBUG: ", "No body, going to CONN_PROCESSING");
					std::string host;
					if (_server) {
						host = _server->getListen().host;
					} else {
						host = "unknown";
					}
					std::ostringstream oss;
					oss << getSocketFd();
					OKlogsEntry("CONNECTION: ", "Receive full request WITHOUT body from socket: " + oss.str() + ", host: " + host);
					setState(CONN_PROCESSING);
				}				
			} else {
				OKlogsEntry("DEBUG: ", "Header not complete yet");
			}
			break;
		}
			
		case CONN_READING_BODY: {
			OKlogsEntry("DEBUG: ", "In CONN_READING_BODY state");
			if (bodyComplete(getReadBuffer())) {
				OKlogsEntry("DEBUG: ", "Body complete, going to CONN_PROCESSING");
				std::string host;
				if (_server) {
					host = _server->getListen().host;
				} else {
					host = "unknown";
				}
				std::ostringstream oss;
				oss << getSocketFd();
				OKlogsEntry("CONNECTION: ", "Receive full request WITH body from socket: " + oss.str() + ", host: " + host);
				setState(CONN_PROCESSING);
			} else {
				OKlogsEntry("DEBUG: ", "Body not complete yet");
			}
			break;
		}

		case CONN_PROCESSING:
			OKlogsEntry("DEBUG: ", "In CONN_PROCESSING state");
			if (getWriteBuffer().empty()) {
				// =================================================================
				// AQUÍ Kevin debe agregar su función de procesamiento de requests:
				// =================================================================
				// Kevin: Llamar tu función aquí así:
				// processRequest(this->getReadBuffer(), this->getServer(), response_output);
				// Luego hacer: this->setWriteBuffer(response_output);
				// =================================================================
				
				// Mostrar el request completo recibido para debug
				OKlogsEntry("HTTP REQUEST: ", "Raw request received:");
				std::cout << "===== HTTP REQUEST START =====" << std::endl;
				std::cout << getReadBuffer() << std::endl;
				std::cout << "===== HTTP REQUEST END =======" << std::endl;
				
				// Respuesta falsa temporal (hasta que Kevin implemente su función)
				std::string fake_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 54\r\n\r\n<html><body><h1>Hello from WebServ</h1></body></html>";
				setWriteBuffer(fake_response);
				startSendingResponse(fake_response);
				
				OKlogsEntry("DEBUG: ", "Response ready, going to CONN_WRITING_RESPONSE");
				setState(CONN_WRITING_RESPONSE);
			}
			break;

		case CONN_WRITING_RESPONSE:
			// El envío se maneja en el event loop con EPOLLOUT
			// Solo verificamos si ya se completó el envío
			if (responseSent()) {
				std::ostringstream oss;
				oss << getSocketFd();
				OKlogsEntry("CONNECTION: ", "Response sent completed on socket: " + oss.str());
				clearBuffers();  // Limpiar buffers para nueva petición
				setState(CONN_KEEP_ALIVE);
			}
			break;

		case CONN_KEEP_ALIVE:
			if (!getReadBuffer().empty()) {
				setState(CONN_READING_HEADER);
			}
			break;

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

void Connection::startSendingResponse(const std::string& response) {
	setResponseComplete(false);
	setBytesToSend(response.size());
	setBytesSent(0);
	setWriteBuffer(response);
}

bool Connection::responseSent() const {
	return _response_complete && (_bytes_sent >= _bytes_to_send);
}

void Connection::updateBytesSent(size_t sent_bytes) {
	_bytes_sent += sent_bytes;
	if (_bytes_sent >= _bytes_to_send) {
		_response_complete = true;
	}
}