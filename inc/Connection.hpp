#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <ctime>
#include <string>

// Forward declarations
class Server;
class HttpResponse; 

enum ConnectionState {
	CONN_READING_REQUEST,
	CONN_PROCESSING_REQUEST,
	CONN_WRITING_RESPONSE,
	CONN_KEEP_ALIVE,
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
	Server*										_server;
	std::string									_read_buff; // request
	std::string									_write_buff; // response
//	HttpResponse*								_response;
	int											_cgi_read_fd;
	int											_cgi_process_pid;

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

	Server* getServer() const;
	void setServer(Server* server);

	const std::string& getReadBuffer() const;
	void appendToReadBuffer(const std::string& data);

	const std::string& getWriteBuffer() const;
	void setWriteBuffer(const std::string& data);
	void appendToWriteBuffer(const std::string& data); // Para streaming de respuestas

//	HttpResponse* getHttpResponse() const;
//	void setHttpResponse(HttpResponse);

	// manejo de conexion
	void clearBuffers();
	void updateActivity();
	bool isTimedOut(time_t current_time, int timeout_seconds) const;
	void closeConnection();
	
	// metodos para el bucle de eventos
	bool isRequestComplete() const;
	bool hasResponseReady() const;
	void processRequest();
	void resetForNextRequest();
	bool responseSent() const;
	void updateBytesSent(size_t sent_bytes);
};

#endif