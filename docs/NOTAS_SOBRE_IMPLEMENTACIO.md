# NOTAS DE TEMAS A REVISAR

- Error Pages: Parsing de múltiples códigos de error (`error_page 500 502 503 504 /50x.html;`)
- Verificar cleanup de `SocketInfo*` y `Connection*`
- Root Path: Normalizar a `/var/www/` automaticamente?
- Se aprsea `alias` en locations?
- Manejo de errores de socket
- Ajustar valores de timeout






**CASOS IDENTIFICADOS:**
  ```nginx
  # Paths relativos (problemáticos)
  root ./html;        # → /var/www/html ?
  root ./;           # → /var/www/ ?
  root ./cgi-bin;    # → /var/www/cgi-bin ?
  
  # Paths absolutos (OK)
  root /var/www/test1;  # ✅ No cambiar
  ```
  
  **DÓNDE NORMALIZAR:**
  - `parseServer()` línea 198: `server.root = tokens[i++];`
  - `parseLocation()` línea 110: `loc.root = tokens[i++];`
  
  **PROPUESTA:** Convertir `./path` → `/var/www/path` solo si no empieza con `/`

- Se aprsea `alias` en locations?
- Manejo de errores de socket
- [ ] **Connection Timeouts**: Ajustar valores de timeout

## 📝 FEATURES FALTANTES

- [ ] **CGI**: Implementar ejecución de scripts CGI
- [ ] **File Upload**: Completar funcionalidad de upload
- [ ] **Directory Listing**: Autoindex con HTML bonito
- [ ] **HTTP Methods**: Completar PUT, DELETE

## 🧪 TESTING NECESARIO

- [ ] **Config Files**: Tests para todas las configs en `/conf/`
- [ ] **Stress Test**: Múltiples conexiones concurrentes
- [ ] **Edge Cases**: Requests malformados
- [ ] **Memory**: Verificar leaks con Valgrind

## 📋 DOCUMENTACIÓN

- [ ] **API Reference**: Documentar interfaz de clases
- [ ] **Architecture**: Diagrama de flujo actualizado
- [ ] **Deployment**: Instrucciones de instalación


## Server name, ports, and hosts.

Nginx first decides which server should process the request.

As an example, many virtual servers with wildcard host and same port. Nginx uses the request 'host' header to
determine which to pick, if there's no match or no header, it uses the default server for that port.

¿Are default servers made by port?
¿Can I have many default servers for one port?
¿Does the default_server directive work?

As an example, if listen has an IP host we have many virtual servers differenciated by server_name. First
the ip and port of the request is tested against the host:port. Then tests the "Host" header like before among the matching host:ports.

¿Default server should be a property of listen port? It should ¿Is it?

How to proceed: https://nginx.org/en/docs/http/server_names.html (Virtual server selection). 

¿If not server name, it's empty.?

¿Does nginx fail if the hostname or ip address doesn't exists? ¿Do we?
¿Does nginx understands a request that comes from either hostname or ip that didn't save? ¿Do we?
¿Do we store the ip and port hit by the client, adn the ip and port of the client? ¿Do we send it to CGI? ¿Do we use to match our servers?

