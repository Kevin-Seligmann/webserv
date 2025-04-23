# Method

## Scope

This document contains the supported methods and the explanation of their role on our server

## Server choices

Supported: GET, HEAD, POST, DELETE.

Unsupported: PUT, OPTIONS, TRACE are unsupported to limit the scope of the project.

Unsupported forever: CONNECT, anything else defined in IANA.

No range requests.

Content on GET/HEAD is rejected with closed connection.

## Content

If a request doens't respect the semantics of the method (Eg. A query that wants to delete something while GET). It must be handled.

### Get

Transfers the representation for the target resource.

It's most common to think of resources as paths on the filesystem and represenations as copy.

### Head

HEAD is a GET but don't send the data. Should send the headers it would send to a GET. It might be worth not to sometimes.

### Post

POST meaning is the one of processing the target resource, can be used for

- Providing a block of data like fields of a form
- Creating a new resource
- Appending data to a new resource

If a resourse is created, the server should send 201 (Created) with a Location header.

### Delete

It deletes a file.

## References

[RFC9110](https://datatracker.ietf.org/doc/html/rfc9110#name-methods) Methods