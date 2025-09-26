#!/usr/bin/env python3
import requests

BASE_URL = "http://localhost:8080"

def test_error_pages():
    print("Testing error pages...")

    endpoints = {
        "/nonexistent": 404,
        "/forbidden": 403,  # debes crear regla/location si quieres probar 403
        "/cgi-bin/nonexistent.py": 500,  # fallo CGI
    }

    for path, expected in endpoints.items():
        try:
            r = requests.get(BASE_URL + path, timeout=3)
            if r.status_code == expected:
                print(f"✓ {path} returned {expected} as expected")
            else:
                print(f"✗ {path} returned {r.status_code}, expected {expected}")
        except Exception as e:
            print(f"✗ {path} request failed: {e}")

if __name__ == "__main__":
    test_error_pages()
