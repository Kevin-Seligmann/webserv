### Notes:

Now there's a fd for epoll_create, a epoll_fd. Somewhere, in a class or in server.

# Classes

This document gives an overview of the class diagram classes. The objetive is to define enough details to make sure that the class diagram will be feasubke to implmenet with the tools c++ provides and the objetives of the project.

This document is to be readed alongisde the class diagram. Redundant information between the document and the class diagram is preferably omitted.

# Class: Server

## Purpose

Entry point of the web server application.

Prepares and owns the VirtualServer s.

Listens on the ports provided by the VirtualServers (Or default).

Handles the event queue and Connection instances.

## Construction

It takes a configuration filename or uses default configuration (Def. Conf. Not defined)

It calls a ConfigurationParser that provides to the server an array of VirtualServer/s, or a default VirtualServer.

If the parsing routine fails, it rethrows to own the exception and let main catch it as a Server exception.

## Method: run() 

1. Calls routine init()
2. Calls routine server_loop()

## Routine: init()

Prepares the necessary sockets to listen to new connections by:

1. Gathering all ports it needs to listen to in a port vector, scanning the virtual servers
2. Bind each port/socket individually and pushing a ListeningSocketPortPair, setting the error boolean and motive if necessary
3. Gather all failed ports/sockets on a vector
4. Printing as an error the message of each failure
5. Removing the failed ports of each virtual server
6. Scanning which virtual servers have no ports left and removing them from the server with an error message
7. Aborting the server with an error message if no virtual server is left

If the routine for listening on a certain port fails, an error is logged for all virtual servers on that port.
If a virtual server lost some ports but it can remain active, it will remain active and a warning will get logged.

As a result, a set of ports and file descriptors pairs is obtained.

8. Call e_poll_create, store the FD
9. Add the file descriptors to e_poll_create with epoll_ctl

## Routine: server_loop()

1. Performs epoll_wait
2. Loop through epoll ready sockets
3. 

If type (In data) is a Connection and it matches the Connection status , it calls process() on Connection.

Process returns ae Event, this must be push into the Event queue.

If it's a listening socket, it tries to open a new Connection.

4. Loop through events

### Event and epoll handling:

For events:

For CLOSING, if grace time expired, destroy the connection, else push into queue again.
For GET_VIRTUAL_SERVER, match the host and port of the connection to the avaiables virtual servers, update Connection
For SET_CGI_INTEREST, or REMOVE_CGI_INTEREST, set or remove the CGI file descriptors from epoll.

For epoll:

When a Connection is born, set the socket fd interest to listen and write.
When a Connection starts to close, remove the fd from the epoll.
For listening sockets, set and unset them at start/end of the program.

## Routine: abort(string: message)

Used for errors that make the web server stop. Logs the message into a Logger as an ERROR, throws an exception.

## Routine: close_listener()

Closes a listening socket that might or not be up and listening. This is mean to serve both the destructor, or any failure on the initialization of the listening connection. Logs an INFO message.

## Destruction

Logs an INFO message saying the server is shut down.

Cleans epoll

Closes all sockets that listen for connections. A failed closing will result in a WARN message informing the user that there might be an issue with their system that caused a close operation to fail.

When Server goes out of scope, so will all Connection. This type of closing depends on Connection and it's abrupt from the perspective of the Server class. This means there's no guaranteed time (By Server) to write response messages informing about the closing, end current responses, or respecting the time gap that Connections give the clients to read responses before closing.

# Class: Connection

Manages the state of I/O operations, each Connection corresponds to a connection received by the server on listen()

Connections hold a state according to their status, it could be

- IDLE - E.g new connections, waitinf for reading on the socket
- READING REQUEST HEADER - On the process of reading a request header, it didn't determine yet what type of request it is
- READING REQUEST BODY - After accepting a regular request, on the process of reading the body
- WRITING RESPONSE - After processing a request, writing on the receiving FD chunk by chunk
- WRITING CGI - Writing to a write side of a PIPE on a CGI
- READING CGI - Reading the read side of a PIPE on a CGI
- READING FILE - Reading a file
- WRITING FILE - Writing to a file
- CLOSING - The Connection started a closing routine
- CLOSED - The Connection has closed and the server must destroy the Connection

These status define routines that take one or more calls from Server. They happen for two reasons: Because some operations require that an FD is non blocking and they must be signaled as such by the Server, or because some operations would take too many resources by themselves and they get chunked. Routines like parsing happen within a single call and don't need to be tracked.

Status should be avaiable to Server, it needs the information to know if it should call process().

## process()

The Server indicates that the Connection must procede with its current non atomic routine. 

This methods returns an struct with information Connection needs to provide to Server.

What is expected from the Server from each Request is:

- GET_VIRTUAL_SERVER - Connection needs a new configuration matching the port and host of the request
- CLOSING - Server must check the timestamp until is time to destroy Connection
- START_CGI - Server needs to know, to put the CGI FDs on epoll
- END_CGI - Server needs to know, to remove the CGI FDs from epoll

The status can be reduced to just 4 options from the Server perspective, that's why the server receives an EventRequest instead of the connection status. EventRequest has the following values: READ_FD, WRITE_FD, CLOSING, CLOSED. READ_FD and WRITE_FD are the same thing, since Connection is just waiting for more information to arrive on the socket.