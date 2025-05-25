# Media Types

## Scope
This document defines the supported MIME types and their course of action.

## Server choices

The server support a small amount of media types listed below, minimally validated according to the rules given on this document.

Parameters on media types are parsed but not used.

Unrecognized media types are interpreted as text/plain and validated as such. 

## Accepted media types

- application/octet-stream ; Binary data
- image/png
- text/html
- text/css
- application/javascript
- application/json
- text/plain ; Text data, default

## Rules

The media type is indicated on the Content-Type header field.

This header has a singleton value

media-type = type "/" subtype parameters
type       = token; Case-ins
subtype    = token; Case-ins

## Media Type specifics

Text-based media types are checked for not having NUL bytes.

PNG media type is checked for PNG signature.

PNG, html, css, and JS are checked for file extension.

No file can have a file extensions belonging to a different media type

## References

[RFC9110-8.3](https://datatracker.ietf.org/doc/html/rfc9110#name-content-type) Content type