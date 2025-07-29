# Intro
The configuration of webserv is based of NGINX. That means it contains simple directives with a name, parameters and ending with a semicolon. And block
directives with a name, parameters and with a block made out of braces. Also called context, when this block can have other directives inside. The
contexts we supprot are named server and location. The analogue to NGINX is to be on the main, http context by default.

# Request processing

First, the a match for a server is found.

# Server name


# Blocks

## Main & Http

This server doens't contain a http directive. It's the default directive, and the context on which all others directives are used.

## Server

The listen directives describe all addresses and ports that accept connections for the server. The server_name directive lists all server names.

## Location

Regex and nested locations are not supported, but they are documented in case they get implemented.

Sets the configuration depending on the request URI. The URI gets normalized and URI-decoded.

Locations are defined by prefixes (URI segments). The location's parameter matches the beggining of the request URI. If there are several matching locations, the one with the longest prefix gets picked.

The '=' character defines an exact match that takes priority above other locations and regex. It doesn't support nested locations.

For CGI requests, when the location has an ending slash and if the location matches up until the last /. The URI must contain this slash. Otherwise a redirect is returned to the correct URI.

Nesting doesn't do anything special. Most directives are not inherited.

# Directives

### error_page: Server, Location

Defines the URI that will be shown on error.

EG: error_page 500 502 503 504 /50x.html; (Implemented ?)

This produces an internal redirect to such URI as a GET. Which means that the context will be the one of the new Location.

(Discutir qué de esto se implementa)

### Listen: Server, Location

Set the address (IP or hostname) and/or port (Def. 80). If not present, all request match. (*:80 or *:8080 (W.o permissions to run on 80))

#### Default server

Makes the server the default server for that address:port. Else, the first server will be default.

### Root: Server, Location

The root directive indicates the actual path on the host filesystem of a request's resource. Which is prepended to the location's URI. 

server {
location /images/ {
    root /data;
}}

A request to /images/example.png points to the resource /data/images/example.png, in this example.

When on server, the root directive has a similar behaviour.

### Server_name: Server

Sets the name of a virtual server, the first is the primary name

server {
    server_name example.com www.example.com;
}

"It allows this server to process requests without the “Host” header field — instead of the default server — for the given address:port pair. This is the default setting" (Deberíamos hacer esto?)

### Index : Server, Location

If the requet ends in a slash, the directive define the files used as index. They are checked in order.

index index.geo.html index.0.html /index.html;

The last can be an absolute path.

This causes an internal redirect when one of the files is found. Which means that the context will be the one of the new Location.

Investigate more, what happens when there's no match.

### Autoindex: Server, Location

If the requet ends in a slash, produces directory listing.

### client_max_body_size: Server, Location  

Max client body size. (Default: 1m)

Would return 413 (Request Entity Too Large). 0 means unlimited size.

### allow_upload: Server?, Location?

Allos POST requests. Default: False.

### upload_dir: Server?, Location?

Restrict uplaods to the specific URI. Default: *.

### Return: Server?, Location?

### allow_methods: Server?, Location?

Methods allowed. Default: GET.



