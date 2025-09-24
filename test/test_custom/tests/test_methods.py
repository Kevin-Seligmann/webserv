#!/usr/bin/env python3
import requests

BASE_URL = "http://localhost:8080"

def test_methods():
    print("Testing allowed and disallowed methods...")

    # Raíz permite GET y POST
    r = requests.get(BASE_URL, timeout=3)
    print(f"GET / -> {r.status_code}")

    r = requests.post(BASE_URL, data={"k": "v"}, timeout=3)
    print(f"POST / -> {r.status_code}")

    r = requests.delete(BASE_URL, timeout=3)
    print(f"DELETE / -> {r.status_code} (expected 405)")

    # /static/ solo permite GET
    r = requests.get(BASE_URL + "/static/", timeout=3)
    print(f"GET /static/ -> {r.status_code}")

    r = requests.post(BASE_URL + "/static/", data={"a": "b"}, timeout=3)
    print(f"POST /static/ -> {r.status_code} (expected 405)")

if __name__ == "__main__":
    test_methods()
