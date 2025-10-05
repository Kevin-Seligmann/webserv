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
	<p>Different types of GCI errores</p>
	<ul>
		<li><a href="cgi_empty.py">CGI returns empty response - Error 502</a></li>
		<li><a href="cgi_no_content_type.py">CGI response doesn't contain header Content-Type - Error 502</a></li>
		<li><a href="cgi_trash.py">CGI doesn't return headers - Error 502</a></li>
		<li><a href="hello.py">CGI can't execute script (doesn't have permission - Error 403</a></li>
    </ul>
</body>
</html>""")
