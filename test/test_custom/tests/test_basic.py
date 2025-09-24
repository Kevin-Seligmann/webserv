#!/usr/bin/env python3
# tests/test_basic.py - Tests básicos del servidor

import requests
import time
import sys
import subprocess

BASE_URL = "http://localhost:8080"
API_URL = "http://localhost:8081"
FILES_URL = "http://localhost:8082"

def test_server_running():
    """Test que el servidor está corriendo"""
    try:
        r = requests.get(BASE_URL, timeout=5)
        print(f"✓ Server running: {r.status_code}")
        return True
    except:
        print("✗ Server not responding")
        return False

def test_static_files():
    """Test de archivos estáticos"""
    tests = [
        (f"{BASE_URL}/", 200),
        (f"{BASE_URL}/index.html", 200),
        (f"{BASE_URL}/static/css/style.css", 200),
        (f"{BASE_URL}/static/js/script.js", 200),
        (f"{BASE_URL}/nonexistent.html", 404),
        (f"{BASE_URL}/errors/403.html", 200),
    ]
    
    for url, expected in tests:
        r = requests.get(url)
        status = "✓" if r.status_code == expected else "✗"
        print(f"{status} GET {url}: {r.status_code} (expected {expected})")

def test_methods():
    """Test de métodos HTTP"""
    # GET
    r = requests.get(f"{BASE_URL}/")
    print(f"✓ GET /: {r.status_code}" if r.status_code == 200 else f"✗ GET /: {r.status_code}")
    
    # POST
    r = requests.post(f"{BASE_URL}/upload/", data={'test': 'data'})
    print(f"✓ POST /upload/: {r.status_code}" if r.status_code in [200, 201] else f"✗ POST /upload/: {r.status_code}")
    
    # DELETE
    r = requests.delete(f"{BASE_URL}/upload/test.txt")
    print(f"✓ DELETE /upload/test.txt: {r.status_code}" if r.status_code in [200, 204, 404] else f"✗ DELETE: {r.status_code}")
    
    # Method not allowed
    r = requests.post(f"{BASE_URL}/static/test.txt", data={'test': 'data'})
    print(f"✓ POST to GET-only location: {r.status_code}" if r.status_code == 405 else f"✗ Method not allowed test: {r.status_code}")

def test_error_pages():
    """Test de páginas de error personalizadas"""
    tests = [
        (f"{BASE_URL}/nonexistent", 404, "404 - Page Not Found"),
        (f"{BASE_URL}/private/secret", 403, "403 - Forbidden"),
    ]
    
    for url, expected_code, expected_text in tests:
        r = requests.get(url)
        has_custom = expected_text in r.text if r.status_code == expected_code else False
        status = "✓" if has_custom else "✗"
        print(f"{status} Custom error page for {expected_code}: {has_custom}")

def test_multiple_ports():
    """Test de múltiples puertos"""
    ports = [
        (BASE_URL, "Webserv"),
        (API_URL, "api"),
        (FILES_URL, "files"),
    ]
    
    for url, expected in ports:
        try:
            r = requests.get(url, timeout=2)
            status = "✓" if r.status_code == 200 else "✗"
            print(f"{status} Server on {url}: {r.status_code}")
        except:
            print(f"✗ Server on {url}: Not responding")

def test_headers():
    """Test de headers HTTP"""
    r = requests.get(BASE_URL)
    
    important_headers = ['Server', 'Date', 'Content-Type', 'Content-Length']
    for header in important_headers:
        if header in r.headers:
            print(f"✓ Header {header}: {r.headers[header][:50]}")
        else:
            print(f"✗ Missing header: {header}")

def test_chunked_encoding():
    """Test de Transfer-Encoding chunked"""
    # Crear request con chunked encoding
    def gen():
        yield b'test'
        yield b'data'
        yield b'chunked'
    
    try:
        r = requests.post(f"{BASE_URL}/upload/", data=gen())
        print(f"✓ Chunked encoding: {r.status_code}")
    except:
        print(f"✗ Chunked encoding failed")

def test_client_max_body_size():
    """Test de tamaño máximo del body"""
    # Crear archivo grande (11MB, más que el límite de 10MB)
    large_data = 'x' * (11 * 1024 * 1024)
    
    r = requests.post(f"{BASE_URL}/upload/", data=large_data)
    if r.status_code == 413:
        print(f"✓ Client max body size enforced: {r.status_code}")
    else:
        print(f"✗ Client max body size not enforced: {r.status_code}")

def test_keep_alive():
    """Test de conexiones Keep-Alive"""
    session = requests.Session()
    
    # Hacer múltiples requests con la misma conexión
    responses = []
    for i in range(3):
        r = session.get(BASE_URL)
        responses.append(r.status_code)
    
    if all(r == 200 for r in responses):
        print(f"✓ Keep-Alive working: {responses}")
    else:
        print(f"✗ Keep-Alive issue: {responses}")

def test_redirect():
    """Test de redirecciones"""
    r = requests.get(f"{BASE_URL}/old-page", allow_redirects=False)
    
    if r.status_code in [301, 302, 303, 307, 308]:
        location = r.headers.get('Location', '')
        print(f"✓ Redirect working: {r.status_code} -> {location}")
    else:
        print(f"✗ Redirect not working: {r.status_code}")

if __name__ == "__main__":
    print("=" * 50)
    print("WEBSERV BASIC TESTS")
    print("=" * 50)
    
    if not test_server_running():
        print("\nERROR: Server not running. Start the server first!")
        sys.exit(1)
    
    print("\n[Static Files]")
    test_static_files()
    
    print("\n[HTTP Methods]")
    test_methods()
    
    print("\n[Error Pages]")
    test_error_pages()
    
    print("\n[Multiple Ports]")
    test_multiple_ports()
    
    print("\n[Headers]")
    test_headers()
    
    print("\n[Advanced Features]")
    test_chunked_encoding()
    test_client_max_body_size()
    test_keep_alive()
    test_redirect()
    
    print("\n" + "=" * 50)
    print("Tests completed!")