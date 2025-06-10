#include "../inc/Connection.hpp"
#include <unistd.h>

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