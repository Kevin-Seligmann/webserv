#!/usr/bin/env python3
# doesn't have content-type header and return 502
print("Status: 200 OK")
print("X-Debug: no-content-type")
print("<html><body><h1>Hello without Content-Type</h1></body></html>")

