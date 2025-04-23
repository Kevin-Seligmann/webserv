# Header

## Scope

This document lists and explains supported fields, header parsing rules and choices made for this web server.

## Server choices

CR, LF or NUL on field-values are considered errors and replaced with SP, other invalid characters are ignored.

Vary and Accept - etc are ignroed

Conditionals are ignored

## Supported Fields

### List

- Host 
- Connection
- Set-Cookie
- Date
- Content-Type
- Content-Length
- Last-Modified
- Allowed
- Location
- Expect

### Host

uri-host [ ":" port ]

### Date

Must be generated in 2xx, 3xx and 4xx responses. And optional in 1xx and 5xx. Ignored on request. (We generate it always)

Read PARSING for date parsing

### Content-Type

Read MEDIA_TYPES for supported media types and details

## Parsing and general rules

### Validation

Ignored or unrecognized header fields must be with correct syntaxis, but their semantic value ignored. 

Incorrect syntaxis implies a 400 message and closed connection

### Field rules

field-line   = field-name ":" OWS field-value OWS; Field name is case-insensitive

No whitespace is allowed between field-name and colon

When a field name is repeated in a section, it's field value is a list of all values field line values present, concatenated as a list.

Never generate new fields when a name is present (Except for Set-Cookie)

Reject field line, values or header/trailer section if they are "too long" (Error 4xx)

Line folding is a way to send multiple values for a line:

obs-fold     = OWS CRLF RWS

Line folding is disallowed except for message/http media type (Could be rejected or replaced by SP)

### Field value rules

Some of the grammar is field dependant (E.g, is it a list?)

Grammar:

field-value    = *field-content
field-content  = field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
field-vchar    = VCHAR / obs-text
obs-text       = %x80-FF

This grammar means that the field value ends and starts with a "field-vchar", and inside there can be any spaces, tabs, or field-vchar

A field value ignores trailing and leading whitespace.

For lists, a field dependant rule is defined where cardinality can be specified: "<n>#<m>element"

Empty elements don't count. Recipients must ignore them to a limit. 

A token is defined as 

  token          = 1*tchar
  tchar          = "!" / "#" / "$" / "%" / "&" / "'" / "*"  / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA ; any VCHAR, except delimiters

Delimiters are chosen from the set of US-ASCII visual characters not allowed in a token (DQUOTE and "(),/:;<=>?@[\]{}").

For sending RWS or OWS, a SP is used, for BWS no space is used.

A string of text can be dquoted and act as a single value:

  quoted-string  = DQUOTE *( qdtext / quoted-pair ) DQUOTE
  qdtext         = HTAB / SP / %x21 / %x23-5B / %x5D-7E / obs-text
  quoted-pair    = "\" ( HTAB / SP / VCHAR / obs-text )

Comments (Not always allowed) are like:

  comment        = "(" *( ctext / quoted-pair / comment ) ")"
  ctext          = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text

Parameters are preceding by a semicolon

parameters      = *( OWS ";" OWS [ parameter ] )
parameter       = parameter-name "=" parameter-value
parameter-name  = token
parameter-value = ( token / quoted-string )

Names are insensitive. They can be dquoted.

There are 3 time formats, all of them have to be accepted but only IMF-fixdate.

IMF-fixdate, RFC 850 format, ANSI C's. [Read for details](https://datatracker.ietf.org/doc/html/rfc9110#name-date-time-formats)

Date is case sensitive

## References

[RFC9110-5](https://datatracker.ietf.org/doc/html/rfc9110#name-fields) Fields
[Time format](https://datatracker.ietf.org/doc/html/rfc9110#name-date-time-formats)
