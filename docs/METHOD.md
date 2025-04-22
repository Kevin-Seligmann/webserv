# Method

## Scope

This document contains the supported methods and the explanation of their role on our server

## Server choices

The supported methods are: GET, HEAD, POST, DELETE.

PUT, OPTIONS, TRACE are unsupported to limit the scope of the project (But could be implemented if it's interesting among other features)

CONNECT is unsupported due to not being a proxy/tunnel/gateway

Anything else defined in IANA is unsupported

No range requests

Content on GET/HEAD is rejected with closed connection

## Content

Unsupported methods are answered with 501. Unsuported method for a specific resource are answered with 405.

If a request doens't respect the semantics of the method (Eg. A query that wants to delete something while GET). It must be handled.

### Get

Transfers the representation for the target resource.

It's most common to think of resources as paths on the filesystem and represenations as copy (Not always the case)

### Head

HEAD is a GET but don't send the data. Should send the headers it would send to a GET. It might be worth not to sometimes.

### Post

POST meaning is the one of processing the target resource, can be used for

- Providing a block of data like fields of a form
- Creating a new resource
- Appending data to a new resource

Status 206, 304 and 416 are not valid responses to POST

If a resourse is created, the server should send 201 (Created) with a Location header.

### Delete

It deletes a file.

Possible responses are: 202 (Accepted: Will likely succeed) 204 (No content: Action done but no more info), 200 (OK: Action done and response includes a representation with the status?)

## References

[RFC9110](https://datatracker.ietf.org/doc/html/rfc9110#name-methods) Methods