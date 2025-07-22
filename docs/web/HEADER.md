# Header

## Index

- Server choices
- Scope
- List of supported headers
- Parsing and general rules
- Supported Fields details
- References

## Scope

This document lists and explains supported fields, header parsing rules and choices made for this web server.

## Server choices

CR, LF or NUL on field-values are considered errors and replaced with SP, other invalid characters are ignored.

Vary and Accept - etc are ignored

Conditionals are ignored


## Request fields

Below there's a short list of fields that are parsed on requests,

- Host
- Content-Length
- Transfer-Encoding
- Connection
- Expect
- Cookie
- Content-Type

## Response fields

Below there's a short list of fields that are generated on responses, and when are they used.

- Allow: When not allowd
- Location: Created responses, Redirections
- Server: = Webserv, always
- Date: Always
- Connection: Close when closing
- Content-Type: When returning content
- Content-Length: When returning content
- last-Modified: For GET requests, before Date
- Transfer-Encoding: = chunked

## Parsing and general rules

### Validation

Ignored or unrecognized header fields must be with correct syntaxis, but their semantic value ignored. 

Incorrect syntaxis implies a 400 message and closed connection

### Field rules

field-line   = field-name ":" OWS field-value OWS; Field name is case-insensitive

field-name = token; Case insensitive and can be dquoted

No whitespace is allowed between field-name and colon

When a field name is repeated in a section, it's field value is a list of all values field line values present, concatenated as a list.

Never generate new fields when a name is present (Except for Set-Cookie)

Reject field line, values or header/trailer section if they are "too long" (Error 4xx)

Line folding is a way to send multiple values for a line:

obs-fold     = OWS CRLF RWS

Line folding is disallowed except for message/http media type (Could be rejected or replaced by SP)

### Field value rules

Some of the grammar is field dependant.

Fields can be singleton (1 Value) or list-based (Multiple values)

Grammar:

field-value    = *field-content
field-content  = field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
field-vchar    = VCHAR / obs-text

This grammar means that the field value ends and starts with a "field-vchar", and inside there can be any spaces, tabs, or field-vchar

A field value ignores trailing and leading whitespace.

Some fields can be defined with delimiters (E.g dquotes)

Delimiters are chosen from the set of US-ASCII visual characters not allowed in a token (DQUOTE and "(),/:;<=>?@[\]{}").

For sending RWS or OWS, a SP is used, for BWS no space is used.

A string of text can be dquoted and act as a single value:

  quoted-string  = DQUOTE *( qdtext / quoted-pair ) DQUOTE
  qdtext         = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
  quoted-pair    = "\" ( HTAB / SP / VCHAR / obs-text )

### Comments

Comments (Not always allowed) are like:

  comment        = "(" *( ctext / quoted-pair / comment ) ")"
  ctext          = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text

Comment avaiability is field dependant

### Parameters

Parameters are preceding by a semicolon.

Parameters names are case-insensitive. Value case depends on field.

parameters      = *( OWS ";" OWS [ parameter ] )
parameter       = parameter-name "=" parameter-value; No whitespace around =
parameter-name  = token
parameter-value = ( token / quoted-string )

### Timestamps

There are 3 time formats, all of them have to be accepted but only IMF-fixdate.

IMF-fixdate, RFC 850 format, ANSI C's. [Read for details](https://datatracker.ietf.org/doc/html/rfc9110#name-date-time-formats)

Date is case sensitive.

Read [RCF9110-5.6.7](https://datatracker.ietf.org/doc/html/rfc9110#section-5.6.7) for more information

### Lists

For lists, a field dependant rule is defined where cardinality can be specified: "<n>#<m>element", meaning at least n at most m

Each element of a list is separated by a comma and OWS

Empty elements don't count. Recipients must ignore them to a limit. 

(This is a grammar rule. Not actually present on lists)

## Supported Fields Details

### Host

uri-host [ ":" port ]

### Date

Must be generated in 2xx, 3xx and 4xx responses. And optional in 1xx and 5xx. Ignored on request. (We generate it always)

Read PARSING for date parsing

### Content-Type

Read MEDIA_TYPES for supported media types and details

## References

[RFC9110-5](https://datatracker.ietf.org/doc/html/rfc9110#name-fields) Fields
[RFC9112-5](https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax) Field Syntax for HTTP/1.1
[Time format](https://datatracker.ietf.org/doc/html/rfc9110#name-date-time-formats)
