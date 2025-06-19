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