#!/usr/bin/env python3
# tests/test_autoindex.py - Test de autoindex

import requests
from bs4 import BeautifulSoup

BASE_URL = "http://localhost:8080"
FILES_URL = "http://localhost:8082"

def test_autoindex_enabled():
    """Test autoindex enabled locations"""
    r = requests.get(f"{BASE_URL}/static/")
    
    if r.status_code == 200:
        soup = BeautifulSoup(r.text, 'html.parser')
        links = soup.find_all('a')
        
        if links:
            print(f"✓ Autoindex enabled: Found {len(links)} items")
            for link in links[:5]:  # Show first 5
                print(f"  - {link.get('href')}")
        else:
            print("✗ Autoindex enabled but no files listed")
    else:
        print(f"✗ Autoindex location returned: {r.status_code}")

def test_autoindex_disabled():
    """Test autoindex disabled locations"""
    r = requests.get(f"{BASE_URL}/")
    
    # Should return index.html, not directory listing
    if "Welcome to Webserv" in r.text:
        print("✓ Autoindex disabled, showing index file")
    else:
        print("✗ Autoindex behavior incorrect")

def test_autoindex_server_level():
    """Test server-level autoindex setting"""
    r = requests.get(FILES_URL)
    
    if r.status_code == 200 and "<a href=" in r.text:
        print("✓ Server-level autoindex working")
    else:
        print("✗ Server-level autoindex not working")

if __name__ == "__main__":
    print("=" * 50)
    print("AUTOINDEX TESTS")
    print("=" * 50)
    
    test_autoindex_enabled()
    test_autoindex_disabled()
    test_autoindex_server_level()
    
    print("\n" + "=" * 50)