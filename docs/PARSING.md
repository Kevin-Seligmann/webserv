# Parsing

## Scope

This document contains parsing rules that are not specific to other sections of the documentation. Most importantly the configuration file parsing and the URI parsing.

## Server choices

The only URI Scheme supported is HTTP

## Content

- Basics
- Token
- URI
- Conf. File

HTTP/1.1 uses ABNF and a List extension defined in RFC 9110.

## Basics

ALPHA          = %x41-5A / %x61-7A  ; A-Z / a-z

CR             = %x0D               ; carriage return

CRLF           = CR LF              ; Internet standard newline

CTL            = %x00-1F / %x7F     ; controls

DIGIT          = %x30-39            ; 0-9

DQUOTE         = %x22               ; " (Double Quote)

HEXDIG         = DIGIT / "A" / "B" / "C" / "D" / "E" / "F"

HTAB           = %x09               ; horizontal tab

LF             = %x0A               ; linefeed

OCTET          = %x00-FF            ; 8 bit character

SP             = %x20               ; Single space

VCHAR          = %x21-7E            ; visible (printing) characters

WSP            = SP / HTAB          ; white space

OWS            = *(WSP)             ; Optional whitespace

RWS            = 1*(WSP)            ; Required whitespace

BWS            = BWS                ; Bad whitespace

obs-text       = %x80-FF            ; Useless text

pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

pct-encoded = "%" HEXDIG HEXDIG

unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"

reserved    = gen-delims / sub-delims

gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"

sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
            / "*" / "+" / "," / ";" / "="
            
## Token

token          = 1*tchar

tchar          = "!" / "#" / "$" / "%" / "&" / "'" / "*"  / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA ; any VCHAR,  except delimiters

## URI

### URI Semantics

In HTTP, "/" and empty path are equivalent. "/" is prefered.

Non reserved characters are percentage-encodable

The "Origin" is the combination of schema, host and port, normalized. Each origin is its own "Entity"

### Syntax

#### Important definitions

http-URI = "http" "://" authority path-abempty [ "?" query ] [ "#" fragment ]; Fragments must be ignored

scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ); Only http is valid (Case insensitive)

authority   = [ userinfo "@" ] host [ ":" port ]; Userinfo presence is an error. Terminated by /, ? or #

segment       = *pchar

host = name or IP; Case insensitive

query       = *( pchar / "/" / "?" ); Terminated by # or end of URI

path = path-abempty 
        / absolute-path

path-abempty  = *( "/" segment )

absolute-path = 1*( "/" segment );

#### Other

absolute-URI  = scheme ":" hier-part [ "?" query ]

partial-URI   = relative-part [ "?" query ]

URI-reference = URI / relative-ref

relative-ref  = relative-part [ "?" query ] [ "#" fragment ]

relative-part = "//" authority path-abempty
            / path-absolute
            / path-noscheme
            / path-empty


## Conf. File

## References

[RFC9110-4.2](https://datatracker.ietf.org/doc/html/rfc9110#section-4.2) HTTP-Related URI Schemes
[RFC3986](https://datatracker.ietf.org/doc/html/rfc3986) URI