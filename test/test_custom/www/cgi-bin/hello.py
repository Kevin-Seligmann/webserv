#python
#!/usr/bin/env python3

import os

print("Content-Type: text/html\r")
print("\r")

print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print(f"<p><strong>Query String:</strong> {os.environ.get('QUERY_STRING', '(empty)')}</p>")
print(f"<p><strong>Path Info:</strong> {os.environ.get('PATH_INFO', '(empty)')}</p>")
print(f"<p><strong>Request Method:</strong> {os.environ.get('REQUEST_METHOD', '(empty)')}</p>")
print(f"<p><strong>Script Name:</strong> {os.environ.get('SCRIPT_NAME', '(empty)')}</p>")
print("</body></html>")