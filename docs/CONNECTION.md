# Connection

## Scope
This document resumes the requirements of connections for HTTP/1.1 and our implementation choices.

## Server choices

This server only acts as a Origin Server (No gateway, proxy or tunnel)

Support HTTP/1.0 Keep-Alive

## Content

## References
------------------

HTTP servers must: maintain the states of connection, establish or reuse connections, progress messages from a connection, detect connection failures and closing connections.

Persistent connections are default: Multiple requests and responses over a single connection. HTTP server should support persistent connections.

Headers can containe a "close" connection option

HTTP/1.1 connection should persist after a response

A server without persistent conneciton must send a "close" on every response

HTTP/1.0 connection should only persist if the request is "Keep-alive"

An upper limit on the number of connections can be established

A timeout value of inactive connections is apropiate. Servers must monitor the connections for a close signal.

It's important (TCP Reset problem) that the server close first the write side. Then mantains the connection until it receives a close. Then close.