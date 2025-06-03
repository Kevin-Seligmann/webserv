# FLUJO COMPLETO: PARSEO → SERVERS

## Arquitectura Consolidada ✅

```
┌─────────────────────────────────────────────────────────────────────┐
│                           MAIN FLOW                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  main() → parseProcess() → serversInit() → socketsInit()           │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## FASE 1: PARSING (main.cpp → Parsed.cpp)

### 1.1 Entrada
```cpp
int main(int argc, char** argv) {
    ParsedServers config;  // vector<ParsedServer>
    
    // STEP 1: Parse configuration file
    if (parseProcess(argc, argv, config) != 0) return 1;
```

### 1.2 Proceso de Parsing
```cpp
int parseProcess(argc, argv, ParsedServers& parsedConfig) {
    // 1. Determinar archivo de configuración
    configFile = (argc == 2) ? argv[1] : "conf/default.conf";
    
    // 2. Leer archivo
    std::ifstream file(configFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    // 3. Tokenizar
    std::vector<std::string> tokens = tokenize(content);
    
    // 4. Parsear estructura
    parsedConfig = parseConfig(tokens);
    
    // 5. Validar configuración
    ServerValidator::validate(parsedConfig);
}
```

### 1.3 Estructura Resultante
```cpp
struct ParsedServer {
    std::vector<ParsedListen> listens;    // MULTIPLE LISTEN DIRECTIVES
    std::vector<std::string> server_names;
    std::string root;
    std::vector<std::string> index_files;
    std::map<int, std::string> error_pages;
    std::map<std::string, ParsedLocations> locations;
    // ... otros campos
};

struct ParsedListen {
    std::string host;       // "localhost", "0.0.0.0", "127.0.0.1"
    int port;              // 8080, 80, 443, etc.
    bool is_default;       // true si tiene "default_server"
};
```

## FASE 2: INICIALIZACIÓN DE SERVERS (initServers.cpp)

### 2.1 Patrón Estilo Nginx ✅
```cpp
bool serversInit(ServersManager& sm, const ParsedServers& ps) {
    
    for (cada ParsedServer config en ps) {
        
        if (config.listens.empty()) {
            // Caso sin listen → usar defaults
            ParsedListen defaultListen;  // 0.0.0.0:8080, no default
            Servers aux_server(config, defaultListen);
            sm.addServer(aux_server.getHostPort(), aux_server);
        } else {
            // Para cada listen directive
            for (cada ParsedListen listen en config.listens) {
                // CREAR UN SERVER POR CADA LISTEN
                Servers aux_server(config, listen);
                sm.addServer(aux_server.getHostPort(), aux_server);
            }
        }
    }
}
```

### 2.2 Lógica Multiplicativa
```
UN BLOQUE SERVER + TRES LISTEN = TRES REGISTROS EN ServersManager

Ejemplo:
server {
    listen 80;
    listen 443 ssl;
    listen 8080 default_server;
    server_name example.com;
    root /var/www;
}

Genera:
├── HostPort(0.0.0.0:80, false) → Servers(config, listen80)
├── HostPort(0.0.0.0:443, false) → Servers(config, listen443)  
└── HostPort(0.0.0.0:8080, true) → Servers(config, listen8080)
```

## FASE 3: ARQUITECTURA DE DATOS

### 3.1 ServersManager
```cpp
struct ServersManager {
    std::map<HostPort, std::vector<Servers>> serversManager;
    //      ^KEY        ^VECTOR DE SERVERS PARA ESE HOST:PORT
};
```

### 3.2 Estructura de Claves
```cpp
struct ParsedListen {  // = HostPort (typedef)
    std::string host;  // Usado como parte de la clave
    int port;         // Usado como parte de la clave  
    bool is_default;  // Metadata del server
    
    // Operadores para usar como clave en std::map
    bool operator<(const ParsedListen& other) const;
    bool operator==(const ParsedListen& other) const;
};
```

### 3.3 Constructor Unificado
```cpp
// Constructor que toma una configuración base y un listen específico
Servers::Servers(const ParsedServer &params, const ParsedListen &listen)
    : _hostPort(HostPort(listen.host, listen.port, listen.is_default))
    , _host(listen.host)           // Del ParsedListen específico
    , _port(listen.port)           // Del ParsedListen específico  
    , _server_name(params.server_names.empty() ? "" : params.server_names[0])
    , _root(params.root)           // De la configuración base
    , _indexFiles(params.index_files)  // De la configuración base
    , _errorPages(params.error_pages)  // De la configuración base
    , _locations(params.locations)     // De la configuración base
    , _default_server(listen.is_default)  // Del ParsedListen específico
{
}
```

## FASE 4: FLUJO VISUAL

```
┌──────────────┐    ┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│ CONFIG FILE  │───▶│   PARSING   │───▶│   SERVERS    │───▶│  SOCKETS    │
│              │    │             │    │              │    │             │
│ server {     │    │ ParsedServer│    │ ServersManager│    │SocketsManager│
│   listen 80; │    │ ├─listens[] │    │ ├─map<HP,S[]>│    │ ├─listen_fds │
│   listen 443;│    │ ├─root      │    │ └─addServer()│    │ └─epoll     │
│   root /var; │    │ └─locations │    │              │    │             │
│ }            │    │             │    │              │    │             │
└──────────────┘    └─────────────┘    └──────────────┘    └─────────────┘
       │                     │                  │                  │
       │                     │                  │                  │
   .conf file          vector<ParsedServer> map<HostPort,        fd[]
   tokenizado          validado y           vector<Servers>>     socket
   y parseado          estructurado         agrupado por        listening
                                           host:port
```

## BENEFICIOS DE LA ARQUITECTURA CONSOLIDADA ✅

1. **Eliminación de Duplicaciones**: `HostPort` = `ParsedListen` unificado
2. **Flujo Nginx-style**: Un server virtual → múltiples puertos  
3. **Validación Centralizada**: `ServerValidator` valida todo antes de crear servers
4. **Manejo de Defaults**: Si no hay `listen`, usa `0.0.0.0:8080`
5. **Compatibilidad**: `typedef HostPort ParsedListen` para código legacy
6. **Sistema de Colores Unificado**: RED, GREEN, YELLOW, BLUE, RESET en toda la aplicación

## TESTING FLOW ✅

```bash
# Compilar
make

# Probar flujo básico
./webserv conf/test_flow.conf

# Verificar salida:
# ├── "Success: parsing completed"
# ├── "Success: servers initialized"  
# ├── Lista de servers configurados
# └── Success exitoso
```

El flujo está **COMPLETAMENTE FUNCIONAL** y **CONSOLIDADO**.
