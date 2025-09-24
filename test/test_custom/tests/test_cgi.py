#!/usr/bin/env python3
# tests/test_cgi.py - Tests de CGI

import requests
import json

BASE_URL = "http://localhost:8080"

def test_cgi_python():
    """Test Python CGI execution"""
    r = requests.get(f"{BASE_URL}/cgi-bin/")
    if "Python CGI is Working!" in r.text:
        print("✓ Python CGI execution working")
    else:
        print("✗ Python CGI not working")
    
    # Test environment variables
    r = requests.get(f"{BASE_URL}/cgi-bin/env.py")
    if "REQUEST_METHOD" in r.text:
        print("✓ CGI environment variables passed")
    else:
        print("✗ CGI environment variables not passed")
    
    # Test GET parameters
    r = requests.get(f"{BASE_URL}/cgi-bin/form.py?name=test&email=test@test.com")
    if "name" in r.text and "test" in r.text:
        print("✓ CGI GET parameters working")
    else:
        print("✗ CGI GET parameters not working")
    
    # Test POST data
    r = requests.post(f"{BASE_URL}/cgi-bin/form.py", data={'username': 'testuser', 'message': 'test message'})
    if "username" in r.text and "testuser" in r.text:
        print("✓ CGI POST data working")
    else:
        print("✗ CGI POST data not working")

def test_cgi_php():
    """Test PHP CGI if configured"""
    try:
        r = requests.get(f"{BASE_URL}/php/info.php")
        if "PHP" in r.text or r.status_code == 200:
            print("✓ PHP CGI working")
        else:
            print("✗ PHP CGI not configured properly")
    except:
        print("⚠ PHP CGI not available")

def test_cgi_upload():
    """Test file upload through CGI"""
    files = {'file': ('test.txt', 'test content', 'text/plain')}
    r = requests.post(f"{BASE_URL}/cgi-bin/upload.py", files=files)
    
    if "File uploaded" in r.text and "test.txt" in r.text:
        print("✓ CGI file upload working")
    else:
        print("✗ CGI file upload not working")

def test_cgi_cookies():
    """Test cookie handling in CGI"""
    session = requests.Session()
    
    # First request sets a cookie
    r1 = session.get(f"{BASE_URL}/cgi-bin/cookie.py")
    
    # Second request should receive the cookie
    r2 = session.get(f"{BASE_URL}/cgi-bin/cookie.py")
    
    if "visit_time" in r2.text:
        print("✓ CGI cookie handling working")
    else:
        print("✗ CGI cookie handling not working")

def test_cgi_timeout():
    """Test CGI timeout handling"""
    # This would need a special slow CGI script
    print("⚠ CGI timeout test requires special slow script")

if __name__ == "__main__":
    print("=" * 50)
    print("CGI TESTS")
    print("=" * 50)
    
    test_cgi_python()
    test_cgi_php()
    test_cgi_upload()
    test_cgi_cookies()
    test_cgi_timeout()
    
    print("\n" + "=" * 50)