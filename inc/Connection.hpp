#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "VirtualServersManager.hpp"
#include "HttpRequest.hpp"
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
	bool										_response_complete;
	size_t										_bytes_to_send;
	size_t										_bytes_sent;
	Servers::VirtualServerInfo*					_server;
	std::string									_read_buff; // request
	HttpRequest*								_request;
	std::string									_write_buff; // response
	HttpResponse*								_response;

public:
	Connection(int fd);
	~Connection();

	int getSocketFd() const;
	void setSocketFd(int fd);

	time_t getLastActivity() const;
	void setLastActivity(time_t last_activity);

	ConnectionState getState() const;
	void setState(ConnectionState state);

	bool getResponseComplete() const;
	void setResponseComplete(bool response_complete);

	size_t getBytesToSend() const;
	void setBytesToSend(size_t bytes_to_send);

	size_t getBytesSent() const;
	void setBytesSent(size_t bytes_sent);

	Servers::VirtualServerInfo* getServer() const;
	void setServer(Servers::VirtualServerInfo* server);

	const std::string& getReadBuffer() const;
	void appendToReadBuffer(const std::string& data);

	HttpRequest* getHttpRequest() const;
	void setHttpRequest(HttpRequest* req);

	const std::string& getWriteBuffer() const;
	void setWriteBuffer(const std::string& data);
	void appendToWriteBuffer(const std::string& data); // Para streaming de respuestas

	HttpResponse* getHttpResponse() const;
	void setHttpResponse(HttpResponse);

	// manejo de conexion
	void clearBuffers();
	void updateActivity();
	void updateConnectionState();
	bool isTimedOut(time_t current_time, int timeout_seconds) const;
	void closeConnection();
	
	// manejo de response
	void startSendingResponse(const std::string& response);
	bool responseSent() const;
	void updateBytesSent(size_t sent_bytes);

};

#endif