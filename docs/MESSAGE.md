# Message

## Scope
This document outlines the basics of HTTP/1.1 messages adapted to this server implementation.

## Server choices

We don't use Content-Location header

Upgrade fields are ignored

We generate dates in every response

No ETag

## Content

### Rules

A message has:

- Control data for routing
- Header lookup table with fields (HEADER.md)
- Stream of content
- Trailer lookup table of fields (Only in chunked encoding)

All responses except 1xx, 204, 304 include (maybe empty) content

HTTP-message   = start-line CRLF
                *( field-line CRLF )
                CRLF
                [ message-body ]
    
start-line     = request-line / status-line; Request for request, status for responses

Plain LF can be a line terminator too.

Bare CR must be replaced with SP on the start-line.

At least one empty lines CRLF (Or more) before the request line can be ignored.

After start-line and before field-lines, whitespace is forbidden.

HTTP-version  = HTTP-name "/" DIGIT "." DIGIT

HTTP-name     = %s"HTTP"

### Request Line

request-line   = method SP request-target SP HTTP-version

Request line could have a size limit, but it should be at least 8000

It's possible (If desired) to have any amount of:  SP, HTAB, VT (%x0B), FF (%x0C), CR instead of a SP. 

method = token

request-target = origin-form / absolute-form

No whitespace is allowed in the request target

A server must respond with 400 to a HTTP/1.1 request without Host header, or more than one Host header, or an invalid Host header value

origin-form    = absolute-path [ "?" query ]

absolute-path = 1*( "/" segment );

absolute-form  = absolute-URI

A server must ignore the Host header, if it's present on the absolute-form

### Response

status-line = HTTP-version SP status-code SP [ reason-phrase ]; Reason often not used

  status-code    = 3DIGIT

The SP follows the same rule as the separators on the request-line

1. If request is HEAD or status is 204 or 304, there's no content (No content-Not modified)
2. If GET and status 200, content is a representation of target resource
3. Else, the content is unidentified by http

Once a request is received and the server determines its target URI, decides.

The server must reject requests if determines that doesn't have the authority to respond, with a 421 status.

### Message data content / Message body

Messages are delimited by either Content-Length or Transfer-Encoding (Read TRANSFER_ENCODING)

Transfer-Encoding overrides Content-Length

Content-Length is valid if it's on a list with all values the same

Data is defined by Content-type and Content-encoding fields.

A server should send a Content-Length if known

The "Content-Type" header field indicates the media type.

Content-Type tags should be generated unless unknown, if there's content.

media-type = type "/" subtype parameters
type       = token
subtype    = token

Type and subtype are case-insensitive

The Content-Encoding field is unsupported and must be answered with 415 (Unsupported Media Type). Same response for unsuported Content-Type

Content-Length indicated the size of the content in octets



## References

[RFC9110](https://datatracker.ietf.org/doc/html/rfc9110#name-message-abstraction) Message Abstraction
[RFC9110-7](https://datatracker.ietf.org/doc/html/rfc9110#name-routing-http-messages) Target routing
[RFC9110-8](https://datatracker.ietf.org/doc/html/rfc9110#name-representation-data-and-met) Content
[RFC9112](https://datatracker.ietf.org/doc/html/rfc9112#name-message) Message HTTP/1.1
