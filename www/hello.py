#!/usr/bin/env python3

import os
import cgi

print("Content-Type: text/html\r\n")
print()
print("<html><body>")
print("<h1>Hello, World! REAL PATH+++</h1>")
print("Query: ", os.environ.get("QUERY_STRING", ""))
print("</body></html>")