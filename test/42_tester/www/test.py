#!/usr/bin/python3
import os
import cgi

print("Content-Type: text/html\r")
print("\r")
print("<h1>Python CGI Test</h1>")
print(f"<p>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'not set')}</p>")
print(f"<p>SCRIPT_NAME: {os.environ.get('SCRIPT_NAME', 'not set')}</p>")
print(f"<p>PATH_INFO: {os.environ.get('PATH_INFO', 'not set')}</p>")
print(f"<p>QUERY_STRING: {os.environ.get('QUERY_STRING', 'not set')}</p>")
print(f"<p>CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', 'not set')}</p>")
print(f"<p>CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', 'not set')}</p>")
print("<p>Python CGI working correctly!</p>")