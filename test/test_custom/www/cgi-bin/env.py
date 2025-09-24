#!/usr/bin/env python3
# cgi-bin/env.py - Mostrar variables de entorno CGI

import os

print("Content-Type: text/html")
print()

print("""<!DOCTYPE html>
<html>
<head>
    <title>CGI Environment Variables</title>
    <style>
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h1>CGI Environment Variables</h1>
    <table>
        <tr><th>Variable</th><th>Value</th></tr>""")

# Important CGI variables
important_vars = [
    'REQUEST_METHOD', 'REQUEST_URI', 'QUERY_STRING', 
    'CONTENT_TYPE', 'CONTENT_LENGTH', 'SERVER_NAME',
    'SERVER_PORT', 'SERVER_PROTOCOL', 'SCRIPT_NAME',
    'PATH_INFO', 'SCRIPT_FILENAME', 'GATEWAY_INTERFACE',
    'REMOTE_ADDR', 'REMOTE_PORT', 'HTTP_HOST',
    'HTTP_USER_AGENT', 'HTTP_ACCEPT', 'HTTP_COOKIE'
]

for var in important_vars:
    value = os.environ.get(var, 'Not Set')
    print(f"<tr><td>{var}</td><td>{value}</td></tr>")

print("""
    </table>
    <h2>All Environment Variables</h2>
    <pre>""")

for key, value in sorted(os.environ.items()):
    print(f"{key} = {value}")

print("""</pre>
    <a href="/">Back to Home</a>
</body>
</html>""")
