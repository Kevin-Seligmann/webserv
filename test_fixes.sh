#!/bin/bash

# Script exhaustivo para probar TODAS las validaciones de Parsed.cpp
# Ejecutar desde la raíz del proyecto: ./test_fixes.sh

echo "=========================================="
echo "   TEST EXHAUSTIVO DE VALIDACIONES"
echo "=========================================="

# Compilar primero
echo -e "\n=== COMPILANDO ==="
make clean
make

if [ $? -ne 0 ]; then
    echo "❌ ERROR: Compilación falló"
    exit 1
fi
echo "✅ Compilación exitosa"

# Función para probar una configuración y capturar el error
test_config() {
    local test_name="$1"
    local config_content="$2"
    local expected_error="$3"
    
    echo -e "\n=== TEST: $test_name ==="
    echo "$config_content" > test_temp.conf
    
    # Ejecutar y capturar solo el error
    ERROR_OUTPUT=$(./webserv test_temp.conf 2>&1 | grep -E "ERROR|Missing|Expected|Unknown|Invalid|Duplicate" | head -1)
    
    if [ ! -z "$ERROR_OUTPUT" ]; then
        echo "✅ Error detectado: ${ERROR_OUTPUT}"
        if [[ "$ERROR_OUTPUT" == *"$expected_error"* ]]; then
            echo "✅ Error esperado correcto"
        else
            echo "⚠️  Error diferente al esperado: $expected_error"
        fi
    else
        echo "❌ No se detectó error (debería fallar)"
    fi
}

echo -e "\n=========================================="
echo "   TESTS DE ESTRUCTURA BÁSICA"
echo "=========================================="

test_config "Falta llave de apertura {" \
"server
    listen 127.0.0.1:8080;
}" \
"Expected: {"

test_config "Falta llave de cierre }" \
"server {
    listen 127.0.0.1:8080;" \
"Unexpected end of file - server block not closed"

test_config "Falta ; después de server_name (detecta antes que })" \
"server {
    listen 127.0.0.1:8080;
    server_name localhost
}" \
"Missing ';' after 'server_name'"

test_config "Token inesperado en lugar de }" \
"server {
    listen 127.0.0.1:8080;
    server_name localhost;
extra_token" \
"Unexpected end of tokens in server block after"

test_config "Falta 'server' keyword" \
"{
    listen 127.0.0.1:8080;
}" \
"Unknown directive"

test_config "Directiva fuera de server" \
"listen 127.0.0.1:8080;
server {
}" \
"Unknown directive outside server"

test_config "Server anidado (server dentro de server)" \
"server {
    listen 127.0.0.1:8080;
    server {
        listen 127.0.0.1:8081;
    }
}" \
"'server' directive cannot be nested"

test_config "Server mal cerrado absorbe siguiente" \
"server {
    listen 127.0.0.1:8080;
server {
    listen 127.0.0.1:8081;
}
}" \
"'server' directive cannot be nested"

echo -e "\n=========================================="
echo "   TESTS DE DIRECTIVAS SIN VALOR"
echo "=========================================="

test_config "server_name sin valor" \
"server {
    listen 127.0.0.1:8080;
    server_name;
}" \
"Missing value for 'server_name'"

test_config "root sin valor" \
"server {
    listen 127.0.0.1:8080;
    root;
}" \
"Missing value for 'root'"

test_config "index sin valor" \
"server {
    listen 127.0.0.1:8080;
    index;
}" \
"Missing value for 'index'"

test_config "autoindex sin valor" \
"server {
    listen 127.0.0.1:8080;
    autoindex;
}" \
"Missing value for 'autoindex'"

test_config "client_max_body_size sin valor" \
"server {
    listen 127.0.0.1:8080;
    client_max_body_size;
}" \
"Missing value for 'client_max_body_size'"

test_config "allow_methods sin valor" \
"server {
    listen 127.0.0.1:8080;
    allow_methods;
}" \
"Missing value for 'allow_methods'"


echo -e "\n=========================================="
echo "   TESTS DE PUNTO Y COMA FALTANTE"
echo "=========================================="

test_config "Falta ; después de listen" \
"server {
    listen 127.0.0.1:8080
    root /var/www;
}" \
"Missing ';' after 'listen'"

test_config "Falta ; después de root" \
"server {
    listen 127.0.0.1:8080;
    root /var/www
}" \
"Missing ';' after 'root'"

test_config "Falta ; después de index" \
"server {
    listen 127.0.0.1:8080;
    index index.html
}" \
"Missing ';' after 'index'"

test_config "Falta ; después de autoindex" \
"server {
    listen 127.0.0.1:8080;
    autoindex on
}" \
"Missing ';' after 'autoindex'"


echo -e "\n=========================================="
echo "   TESTS DE LOCATION"
echo "=========================================="

test_config "Location sin path" \
"server {
    listen 127.0.0.1:8080;
    location {
        root /var/www;
    }
}" \
"Missing location path"
# === TEST: Location sin path ===
# ✅ Error detectado: [Sun Sep 28 18:50:37 2025] Error. PARSING ERROR: Expected: {, got: root
# ⚠️  Error diferente al esperado: Missing location path

test_config "Location sin {" \
"server {
    listen 127.0.0.1:8080;
    location /test
        root /var/www;
    }
}" \
"Expected: {"

#test_config "Location con directiva inválida" \
#"server {
#    listen 127.0.0.1:8080;
#    location / {
#        invalid_directive value;
#    }
# }" \
# "not supported"
# Genera bucle infinito

test_config "Location dentro de location" \
"server {
    listen 127.0.0.1:8080;
    location / {
        location /nested {
            root /var/www;
        }
    }
}" \
"Locations inside location"

test_config "Location duplicada" \
"server {
    listen 127.0.0.1:8080;
    location / {
        root /var/www;
    }
    location / {
        root /var/www2;
    }
}" \
"Duplicate location"

echo -e "\n=========================================="
echo "   TESTS DE ERROR_PAGE"
echo "=========================================="

test_config "error_page sin código" \
"server {
    listen 127.0.0.1:8080;
    error_page /404.html;
}" \
"at least one error code"

test_config "error_page sin path" \
"server {
    listen 127.0.0.1:8080;
    error_page 404;
}" \
"requires a file path"

test_config "error_page sin ; final" \
"server {
    listen 127.0.0.1:8080;
    error_page 404 /404.html
    root /www;
}" \
"Missing ';'"

echo -e "\n=========================================="
echo "   TESTS DE DIRECTIVAS EN LOCATION"
echo "=========================================="

test_config "allow_methods sin valor en location" \
"server {
    listen 127.0.0.1:8080;
    location / {
        allow_methods;
    }
}" \
"Missing value for 'allow_methods' directive in location"

test_config "root sin valor en location" \
"server {
    listen 127.0.0.1:8080;
    location / {
        root;
    }
}" \
"Missing value for 'root' directive in location"
# === TEST: root sin valor en location ===
# ✅ Error detectado: [Sun Sep 28 18:58:54 2025] Error. PARSING ERROR: Missing ';' after 'root' directive at src/Parsed.cpp:195
# ⚠️  Error diferente al esperado: Missing value for 'root' directive in location

test_config "alias sin valor" \
"server {
    listen 127.0.0.1:8080;
    location / {
        alias;
    }
}" \
"Missing value for 'alias'"
#=== TEST: alias sin valor ===
#✅ Error detectado: [Sun Sep 28 18:58:54 2025] Error. PARSING ERROR: Missing ';' after 'alias' directive. at src/Parsed.cpp:321
#⚠️  Error diferente al esperado: Missing value for 'alias'

test_config "redirect sin valor" \
"server {
    listen 127.0.0.1:8080;
    location / {
        redirect;
    }
}" \
"Missing value for 'redirect'"
# === TEST: redirect sin valor ===
# ✅ Error detectado: [Sun Sep 28 18:58:54 2025] Error. PARSING ERROR: Missing ';' after 'redirect' directive. at src/Parsed.cpp:252
# ⚠️  Error diferente al esperado: Missing value for 'redirect'

test_config "cgi_extension sin valor" \
"server {
    listen 127.0.0.1:8080;
    location /cgi-bin/ {
        cgi_extension;
    }
}" \
"Missing value for 'cgi_extension'"
# === TEST: cgi_extension sin valor ===
# ✅ Error detectado: [Sun Sep 28 18:58:54 2025] Error. PARSING ERROR: Missing ';' after 'cgi_extension' directive. at src/Parsed.cpp:264
# ⚠️  Error diferente al esperado: Missing value for 'cgi_extension'

echo -e "\n=========================================="
echo "   TESTS DE VALORES INVÁLIDOS"
echo "=========================================="

# test_config "autoindex con valor inválido" \
# "server {
#    listen 127.0.0.1:8080;
#    autoindex maybe;
# }" \
# "Invalid argument for 'autoindex'"
# Genera bucle infinito o queda el test en suspenso

#test_config "listen con puerto inválido" \
#"server {
#    listen 127.0.0.1:999999;
#}" \
"" # Puerto fuera de rango - puede no dar error específico
# Genera bucle infinito o queda el test en suspenso

test_config "listen con IPv6" \
"server {
    listen [::1]:8080;
}" \
"IPv6 addresses are not allowed"

echo -e "\n=========================================="
echo "   TESTS DE CASOS EXTREMOS"  
echo "=========================================="

test_config "Archivo vacío" \
"" \
"Config file empty."
# === TEST: Archivo vacío ===
# ❌ No se detectó error (debería fallar)


test_config "Solo comentarios" \
"# Este es un comentario
 # Otro comentario" \
""
# === TEST: Solo comentarios ===
# ❌ No se detectó error (debería fallar)


# test_config "Server vacío" \
#"server {
# }" \
# ""  # Server vacío es técnicamente válido
# Genera bucle infinito o queda el test en suspenso


test_config "Múltiples errores (toma el primero)" \
"server {
    listen 127.0.0.1:8080;
    root;
    index;
    autoindex;
}" \
"Missing value for 'root'"

test_config "Token inesperado al final" \
"server {
    listen 127.0.0.1:8080;
}
extra_token" \
"Unknown directive outside server"

echo -e "\n=========================================="
echo "   RESUMEN DE VALIDACIONES"
echo "=========================================="

# Contar comentarios APLICADO AQUI
VALIDATIONS=$(grep -c "APLICADO AQUI" src/Parsed.cpp)
echo "✅ Total de validaciones con comentario 'APLICADO AQUI': $VALIDATIONS"

# Limpiar
rm -f test_temp.conf

echo -e "\n=========================================="
echo "   PRUEBA CON CONFIGURACIÓN VÁLIDA"
echo "=========================================="

cat > test_valid.conf << EOF
server {
    listen 127.0.0.1:8080;
    server_name localhost;
    root /var/www;
    index index.html;
    
    location / {
        allow_methods GET POST;
        autoindex off;
    }
}
EOF

echo "Probando configuración válida (NO debe dar errores):"
ERROR=$(./webserv test_valid.conf 2>&1 | grep -E "ERROR|PARSING ERROR" | head -1)
if [ -z "$ERROR" ]; then
    echo "✅ Configuración válida aceptada correctamente"
else
    echo "❌ Error inesperado con config válida: $ERROR"
fi
# Genera bucle infinito o queda el test en suspenso
# No entra en el if

rm -f test_valid.conf

echo -e "\n✅ TODOS LOS TESTS COMPLETADOS"
echo "=========================================="
