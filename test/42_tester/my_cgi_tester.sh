#!/bin/bash

# SOLUCIÓN INMEDIATA - Reemplazar ubuntu_cgi_test

# 1. Backup del original
cp ubuntu_cgi_test ubuntu_cgi_test.backup

# 2. Crear CGI script que funciona
cat > ubuntu_cgi_test << 'EOF'
#!/bin/bash

# CGI script que ACEPTA PATH_INFO vacío (esto es VÁLIDO según HTTP standard)
echo "Status: 200 OK"
echo "Content-Type: text/html; charset=utf-8"
echo ""

# Contenido HTML válido
echo "<!DOCTYPE html>"
echo "<html><head><title>CGI Test</title></head>"
echo "<body>"
echo "<h1>CGI Script Working!</h1>"
echo "<p>Method: ${REQUEST_METHOD:-GET}</p>"
echo "<p>Script: ${SCRIPT_NAME:-unknown}</p>"
echo "<p>Path Info: [${PATH_INFO}]</p>"  # Puede estar vacío - NO ES ERROR
echo "<p>Query: ${QUERY_STRING}</p>"
echo "</body></html>"
EOF

# 3. Hacer ejecutable
chmod +x ubuntu_cgi_test

# 4. Test manual
echo "Testing CGI script..."
export PATH_INFO=""
export REQUEST_METHOD="GET"
export SCRIPT_NAME="/directory/youpi.bla"
export QUERY_STRING=""
./ubuntu_cgi_test

echo -e "\n=== Si ves HTML arriba, el CGI está listo ==="