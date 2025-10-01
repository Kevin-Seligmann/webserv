#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs
from html import escape

method = os.environ.get('REQUEST_METHOD', 'GET')
form_data = {}

if method == 'GET':
    query_string = os.environ.get('QUERY_STRING', '')
    form_data = parse_qs(query_string)
elif method == 'POST':
    content_type = os.environ.get('CONTENT_TYPE', '')
    
    # Solo aceptar urlencoded para este script
    if 'application/x-www-form-urlencoded' not in content_type:
        print("Content-Type: text/plain\r")
        print("\r")
        print(f"Error: Expected urlencoded, got {content_type}")
        print("Use /cgi-bin/upload.py for file uploads")
        sys.exit(0)
    
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        form_data = parse_qs(post_data, keep_blank_values=True)

print("Content-Type: text/html; charset=utf-8\r")
print("\r")

print("""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Form Handler</title>
</head>
<body>
    <h1>Form Data</h1>""")

print(f"<p><strong>Method:</strong> {method}</p>")

if form_data:
    print("<h2>Fields:</h2><dl>")
    for key, values in form_data.items():
        # parse_qs devuelve listas, mostrar todos los valores
        for value in values:
            # Escapar HTML para prevenir XSS
            safe_key = escape(key)
            safe_value = escape(value)
            print(f"<dt><strong>{safe_key}:</strong></dt>")
            print(f"<dd>{safe_value}</dd>")
    print("</dl>")
else:
    print("<p>No data received</p>")

# Debug info
if method == 'GET':
    qs = os.environ.get('QUERY_STRING', '')
    if qs:
        print(f"<p><small>Query: {escape(qs)}</small></p>")

print("""
    <hr>
    <h3>Test Form</h3>
    <form method="POST">
        <input name="name" placeholder="Name" required>
        <input name="email" type="email" placeholder="Email">
        <button type="submit">Submit</button>
    </form>
    <a href="/">Back</a>
</body>
</html>""")