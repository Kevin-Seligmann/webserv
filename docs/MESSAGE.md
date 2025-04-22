# Message

## Scope
This document outlines the basics of HTTP/1.1 messages adapted to the this server implementation.

## Server choices

We don't use Content-Location header

Trailers are unsuppored (Ignored)

Upgrade fields are ignored

We generate dates in every response

HEAD responses with no Content-Length

No ETag

## Content

### Rules

A message has a control data for routing

A header lookup table with fields (Read HEADER.md)

Stream of content

A trailer lookup table of fields, must be parsed and ignored.

A message its a stream of octets after the head delineated by the "Message framing"

All responses expect 1xx, 204, 304 include (maybe empty) content


### Response

1. If request is HEAD or status is 204 or 304, there's no content (No content-Not modified)
2. If GET and status 200, content is a representation of target resource
3. Else, the content is unidentified by http

Once a request is received and the server determines its target URI, decides.

The server must reject requests if determines that doesn't have the authority to respond, with a 421 status.

### Message data content

Data is defined by Content-type and Content-encoding fields.

The "Content-Type" header field indicates the media type.

Content-Type tags should be generated unless unknown, if there's content.

media-type = type "/" subtype parameters
type       = token
subtype    = token

Type and subtype are case-insensitive

The Content-Encoding field is unsupported and must be answered with 415 (Unsupported Media Type). Same response for unsuported Content-Type

Content-Length indicated the size of the content in octets

A server should send a Content-Length if known

## References

[RFC9110](https://datatracker.ietf.org/doc/html/rfc9110#name-message-abstraction) Message Abstraction
[RFC9110-7](https://datatracker.ietf.org/doc/html/rfc9110#name-routing-http-messages) Target routing
[RFC9110-8](https://datatracker.ietf.org/doc/html/rfc9110#name-representation-data-and-met) Content