# Transfer Encoding

## Scope
This document defines the syntax and behaviour of transfer encoding

## Server choices

Only chunked is supported

## Tramsfer Encoding

Transfer-Encoding header field lists the transfer coding names of the transfer coding applied to the body.

Transfer-Encoding = #transfer-coding; (e.g Transfer-Encoding = chunked)

When the server doesn't understand a coding, should reply with 501.

A server must not reply with Transfer-Encoding to other than HTTP/1.1

A server must close a connection with both length and transfer. Regardless of response.

Coding names are case insensitive

## Chunked

Chunked transfer sends the content as a serie of chunks, which its own size indicator and an optional trailer section.

Chunked coding has no parameters and their presence should be an error

### Chunk extensions

chunk-ext is a chunk extension with metadata, or even random content.

Unrecognized chunk-ext should be ignored.

### Trailer section

For this server, should be parsed and ignored.

### TE Header field

It's unnecesary. Used for a client to tell which transfers it accepts. Chunked is always accepted for HTTP/1.1.

### Grammar

chunked-body   = *chunk
                last-chunk
                trailer-section
                CRLF

chunk          = chunk-size [ chunk-ext ] CRLF
                chunk-data CRLF

chunk-size     = 1*HEXDIG; Size in octets

last-chunk     = 1*("0") [ chunk-ext ] CRLF; Chunk size of 0 is the last chunk

chunk-data     = 1*OCTET 

chunk-ext      = *( BWS ";" BWS chunk-ext-name [ BWS "=" BWS chunk-ext-val ] )

chunk-ext-name = token

chunk-ext-val  = token / quoted-string

## References

[RFC9112-8](https://datatracker.ietf.org/doc/html/rfc9112#name-transfer-encoding) Transfer Encoding
[RFC9112-7](https://datatracker.ietf.org/doc/html/rfc9112#name-transfer-codings) Transfer Codings
[RFC9110-10.1.4](https://datatracker.ietf.org/doc/html/rfc9110#name-te) TE Header field