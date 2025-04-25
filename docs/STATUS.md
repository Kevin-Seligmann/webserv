# Status

## Scope
This document contains a list of supported status, their meaning and their use cases.

## Server choices

## Content

## 1xx: Information
Should not be sent to HTTP/1.0.

It doesn't contain content.

### 100: Continue 
A response to an request with an Expect field, if the response has not been rejected.

## 2xx: Successful
Received, understood and accepted.

### 200: Ok
Content is:

- For GET, target resource
- For HEAD is like GET, but without data
- For POST is the status or results obtained
- For DELETE is the status of the action

### 201: Created
Request fulfilled and resource created.

Resource is identified by a Location header. 

If no Location, is identified by the target URI.

### 204: No content
Successful request but there's no content to send in the response.

## 3xx: Information

### 302: Found

Answer to a CGI Client Redirect Response (Read CGI)

## 4xx: Client error
The server should send a representation with an explanation of the error and if it's temporary or permanent.

### 400: Bad request
The server cannot or will not process the request due to a client error: Wrong syntax, invalid request framing, deceptive request routing.

### 404: Not found
A not found resource or unwillingness to indicate a resource exists or not.

### 405: Method not allowed
The method is recognized but not allowed on this resource. 

Must be replied with a Allow header.

### 408: Request timeout

### 411: Length Required
Client must add Content-Length or other kind of content framing.

### 413: Content too large

### 414: URI too long

### 415: Unsupported Media Type

### 417: Expectation failed
The Expect header can't be meet (No continue).

### 421: Misdirected Request
Unable or unwilling to produce an answer for the target URI.

## 5xx: Server error
Except for head, a server should send a message explaining if it's temporary or permanent, and what

### 500: Interval server error
Unexpected internal error. Send an explanation of the error and if it's temporary or permanent.

### 501: Not implemented
When a server doesn't recognize the request method. (Or a funcionallity).

### 503: Service unavaiable
The server is overloaded in some form.

Possible to send to new connections before rejecting them.

### 505: HTTP Version not supported

## References

[RFC9110-15](https://datatracker.ietf.org/doc/html/rfc9110#name-status-codes) Status codes