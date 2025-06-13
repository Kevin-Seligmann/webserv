#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "VirtualServersManager.hpp"
#include <ctime>
#include <string> 

enum ConnectionState {
	CONN_IDLE,
	CONN_READING_HEADER,
	CONN_READING_BODY,
	CONN_PROCESSING,
	CONN_WRITING_RESPONSE,
	CONN_KEEP_ALIVE,
	CONN_CLOSING,
	CONN_CLOSED
};

class Connection {
private:
	int											_socket_fd;
	time_t										_last_activity;
	ConnectionState								_state;
	std::string									_read_buff; // request
	std::string									_write_buff; // response
	bool										_response_complete;
	size_t										_bytes_to_send;
	size_t										_bytes_sent;
	VirtualServersManager::VirtualServerGroup*	_server;

public:
	Connection(int fd);
	~Connection();

	int getSocketFd() const;
	void setSocketFd(int fd);
	
	time_t getLastActivity() const;
	void setLastActivity(time_t last_activity);

	ConnectionState getState() const;
	void setState(ConnectionState state);

	const std::string& getReadBuffer() const;
	void appendToReadBuffer(const std::string& data);
	
	const std::string& getWriteBuffer() const;
	void setWriteBuffer(const std::string& data);

	void clearBuffers();

	void updateActivity();
	void updateConnectionState();
	bool isTimedOut(time_t current_time, int timeout_seconds) const;

	void closeConnection();

};

#endif