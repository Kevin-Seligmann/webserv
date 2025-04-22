# Parsing

## Scope

This document contains parsing rules that are not specific to other sections of the documentation. Most importantly the configuration file parsing and the URI parsing.

## Server choices

The only URI Scheme supported is HTTP

## Content

[RFC9110-4.2]

An complete HTTP URI is defined as:  http-URI = "http" "://" authority path-abempty [ "?" query ]

The origin server is identified by the authority's host identifier. If port is not defined, TCP 80 is the default.

(Unless OPTIONS request): An empty path is equivalent to "/"

Scheme and host are case-insensitive, the rest case sensitive

Unreserved characters are equivalent to percent-encoded octets

The server should parse the userinfo (@ part of an URI) and report an error

Fragment identifiers (For example) are unsupported, everything after # is discarted

The URI origin is defined as schema + host + port. (E.g http://example.com:443) 


### URI


### Conf. File

## References

[RFC9110-4.2](https://datatracker.ietf.org/doc/html/rfc9110#section-4.2) HTTP-Related URI Schemes
