#!/usr/bin/env python3
# tests/test_stress.py - Stress testing

import requests
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

BASE_URL = "http://localhost:8080"

def make_request(url, session=None):
    """Make a single request"""
    try:
        if session:
            r = session.get(url, timeout=5)
        else:
            r = requests.get(url, timeout=5)
        return r.status_code
    except Exception as e:
        return f"Error: {str(e)}"

def test_concurrent_connections():
    """Test multiple concurrent connections"""
    print("Testing 100 concurrent connections...")
    
    with ThreadPoolExecutor(max_workers=100) as executor:
        futures = []
        for i in range(100):
            future = executor.submit(make_request, BASE_URL)
            futures.append(future)
        
        results = {}
        for future in as_completed(futures):
            result = future.result()
            results[result] = results.get(result, 0) + 1
    
    print(f"Results: {results}")
    
    if results.get(200, 0) > 90:  # At least 90% success
        print("✓ Server handles concurrent connections well")
    else:
        print("✗ Server struggles with concurrent connections")

def test_sustained_load():
    """Test sustained load over time"""
    print("Testing sustained load (30 seconds)...")
    
    start_time = time.time()
    request_count = 0
    error_count = 0
    
    while time.time() - start_time < 30:
        try:
            r = requests.get(BASE_URL, timeout=2)
            if r.status_code != 200:
                error_count += 1
            request_count += 1
        except:
            error_count += 1
            request_count += 1
        
        if request_count % 100 == 0:
            print(f"  {request_count} requests sent, {error_count} errors")
    
    success_rate = (request_count - error_count) / request_count * 100
    print(f"\nTotal: {request_count} requests, {error_count} errors")
    print(f"Success rate: {success_rate:.2f}%")
    
    if success_rate > 95:
        print("✓ Server remains stable under sustained load")
    else:
        print("✗ Server stability issues under load")

def test_large_file_download():
    """Test downloading large files"""
    # This assumes you have a large file in static/files/
    print("Testing large file download...")
    
    try:
        r = requests.get(f"{BASE_URL}/static/files/document.pdf", stream=True, timeout=30)
        total_size = 0
        
        for chunk in r.iter_content(chunk_size=8192):
            total_size += len(chunk)
        
        print(f"✓ Downloaded {total_size / 1024 / 1024:.2f} MB successfully")
    except Exception as e:
        print(f"✗ Large file download failed: {e}")

def test_keep_alive_stress():
    """Test Keep-Alive under stress"""
    print("Testing Keep-Alive with 50 requests on single connection...")
    
    session = requests.Session()
    success_count = 0
    
    for i in range(50):
        try:
            r = session.get(BASE_URL, timeout=2)
            if r.status_code == 200:
                success_count += 1
        except:
            pass
    
    if success_count == 50:
        print(f"✓ Keep-Alive stable: {success_count}/50 successful")
    else:
        print(f"✗ Keep-Alive issues: {success_count}/50 successful")

if __name__ == "__main__":
    print("=" * 50)
    print("STRESS TESTS")
    print("=" * 50)
    print("WARNING: These tests will put load on your server!")
    print()
    
    test_concurrent_connections()
    print()
    test_sustained_load()
    print()
    test_large_file_download()
    print()
    test_keep_alive_stress()
    
    print("\n" + "=" * 50)
    print("Stress tests completed!")