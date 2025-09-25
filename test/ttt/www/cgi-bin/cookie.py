# cgi-bin/cookie.py - Testing de cookies

#!/usr/bin/env python3
import os
from http import cookies
import time

# Get existing cookies
cookie_string = os.environ.get('HTTP_COOKIE')
c = cookies.SimpleCookie(cookie_string)

# Set a new cookie
new_cookie = cookies.SimpleCookie()
new_cookie['visit_time'] = str(int(time.time()))
new_cookie['visit_time']['path'] = '/'
new_cookie['visit_time']['max-age'] = 3600

print(new_cookie.output())
print("Content-Type: text/html")
print()

print("""<!DOCTYPE html>
<html>
<head>
    <title>Cookie Test</title>
</head>
<body>
    <h1>Cookie Testing</h1>""")

if cookie_string:
    print(f"<p><strong>Received cookies:</strong> {cookie_string}</p>")
    print("<h2>Parsed Cookies:</h2><ul>")
    for key in c.keys():
        print(f"<li>{key} = {c[key].value}</li>")
    print("</ul>")
else:
    print("<p>No cookies received.</p>")

print(f"""
    <p>New cookie set: visit_time = {new_cookie['visit_time'].value}</p>
    <hr>
    <a href="cookie.py">Reload to see cookies</a><br>
    <a href="/">Back to Home</a>
</body>
</html>""")