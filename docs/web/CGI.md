# CGI

Important:

To be reviewed to narrow the scope.

Status of the document: 

CGI will be implemented after other basic functionality. Expanding to implement CGI should not produce significant changes on the rest of the code (Except maybe shared utilities for parsing). As a result, this document is incomplete util more details are defined.

## Scope
This document resumes RFC3875 (CGI) for this implementation of a web server

## Server choices

## Content

CGI allows an HTTP server and a "CGI script" to co-manage clients' requests.

CGI defines parameters or meta-variables which describe a client request.

A 'meta-variable' is a parameter from the server to the script. (Usually as an ENV. Variable).

A 'script' is a board term for the handling sofware (An actual 'script', a library, the server itself, etc).

Elements are case-insensitive by fault.

[RFC3875](https://datatracker.ietf.org/doc/html/rfc3875#section-2.2) contains specifics about grammar

The server recieves the request, selects a CGI script, creates a CGI request, executes the script, and transforms the CGI response into an HTTP response.

The selection of script is dependant on the URI

The URI includes a path. All or the leading part of the path is identified with an script. The remained of the path is the resource for this script.

The mapping from URI to script is server-dependant.

The script might be identified by many different URIs.

The server might exchange the URI by an equivalent during generation of meta-variables.

The server could construct a URI from meta-variables. (The Script-URI). And if the client accessed this URI, the script would've been executed with the same values for the meta-variables SCIRPT_NAME, PATH_INFO and QUERY_STRING

The Script-URI is a generic URI without fragment identifiers.

script-URI = <scheme> "://" <server-name> ":" <server-port> <script-path> <extra-path> "?" <query-string>

where server-name, server-port, query-string are the values of their respectives meta-variables.

The SCRIPT_NAME and PATH_INFO values are URL-encoded (;, =, ? reserved), and give <script-path> and <extra-path>

## Request

The information of request comes from meta-variables and message body.

meta-variables are accessed thorugh env-variables on this implementation

Some variables:

meta-variable-name = "AUTH_TYPE" | "CONTENT_LENGTH" |
                    "CONTENT_TYPE" | "GATEWAY_INTERFACE" |
                    "PATH_INFO" | "PATH_TRANSLATED" |
                    "QUERY_STRING" | "REMOTE_ADDR" |
                    "REMOTE_HOST" | "REMOTE_IDENT" |
                    "REMOTE_USER" | "REQUEST_METHOD" |
                    "SCRIPT_NAME" | "SERVER_NAME" |
                    "SERVER_PORT" | "SERVER_PROTOCOL" |
                    "SERVER_SOFTWARE" | scheme |
                    protocol-var-name | extension-var-name
protocol-var-name  = ( protocol | scheme ) "_" var-name
scheme             = alpha *( alpha | digit | "+" | "-" | "." )
var-name           = token

And some protocol dependant variables are defined (HTTP_)

And internal variables (X_)

meta-variable-value = "" | 1*<TEXT, CHAR or tokens of value>

Values are case sensitive by default

## Request Variables

Variables are script-dependant. They will be better defined when the used CGIs are defined.

Documenting them here would fill the documentaiton with unused information.

The explanation of all of them can be found on the RFC CGI document.

## Request message-body

Request data is accessed in a system-defined method. And by default reading stdin.

Request-Data   = [ request-body ] [ extension-data ]
request-body   = <CONTENT_LENGTH>OCTET
extension-data = *OCTET

## CGI Response

Be default, response is read from STDOUT.

The server might implement a timeout.

The response comprises a message-header and a message-body. The message-header contains one or more header
fields. The body may be NULL.

generic-response = 1*header-field NL [ response-body ]

The script can return a document, local redirect or client redirect.

### Document Response

document-response = Content-Type [ Status ] *other-field NL
                    response-body

If status is not present, 200 is assumed.

### Local Redirect

The script can return a URI and query-string for a local resource in a Location header field.

The script must not return other header fields or message body.

The server must generate a response equivalent to a request to that path

### Client Redirect

The script can return an absolute URI path in a Location header. Information for the client.

The server must generate a '302' found response.

client-redir-response = client-Location *extension-field NL

### Client Redirect with Document

client-redirdoc-response = client-Location Status Content-Type
                            *other-field NL response-body

### Response header fields

At least one CGI-Field must be supplied, each field must not appear more than once.

header-field    = CGI-field | other-field
CGI-field       = Content-Type | Location | Status
other-field     = protocol-field | extension-field
protocol-field  = generic-field
extension-field = generic-field
generic-field   = field-name ":" [ field-value ] NL
field-name      = token
field-value     = *( field-content | LWSP )
field-content   = *( token | separator | quoted-string )

Read RFC

## References

[RFC3875](https://datatracker.ietf.org/doc/html/rfc3875) CGI