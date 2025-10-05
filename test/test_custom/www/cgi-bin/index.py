#!/usr/bin/env python3
# cgi-bin/index.py - CGI principal de testing

import os
import sys

print("Content-Type: text/html")
print()
print("""<!DOCTYPE html>
<html>
<head>
    <title>CGI Python Test</title>
</head>
<body>
    <h1>Python CGI</h1>
    <p>If you can see this, CGI execution is functioning correctly.</p>
    <ul>
        <li><a href="form.py">Form Test</a></li>
        <li><a href="env.py">Environment Variables</a></li>
        <li><a href="upload.py">Upload Test</a></li>
        <li><a href="cookie.py">Cookie Test</a></li>
    </ul>
</body>
</html>""")