#!/usr/bin/env python3
# cgi-bin/form.py - Manejo de formularios GET/POST

import cgi
import os

# Parse form data
form = cgi.FieldStorage()

print("Content-Type: text/html")
print()

print("""<!DOCTYPE html>
<html>
<head>
    <title>Form Processing</title>
</head>
<body>
    <h1>Form Data Received</h1>""")

# Show request method
method = os.environ.get('REQUEST_METHOD', 'Unknown')
print(f"<p><strong>Method:</strong> {method}</p>")

# Show form fields
if form:
    print("<h2>Form Fields:</h2><ul>")
    for key in form.keys():
        value = form.getvalue(key)
        print(f"<li><strong>{key}:</strong> {value}</li>")
    print("</ul>")
else:
    print("<p>No form data received.</p>")

# Show query string for GET
if method == "GET":
    query = os.environ.get('QUERY_STRING', '')
    print(f"<p><strong>Query String:</strong> {query}</p>")

print("""
    <hr>
    <a href="/">Back to Home</a>
</body>
</html>""")