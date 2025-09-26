#!/usr/bin/env python3
# cgi-bin/upload.py - Manejo de uploads

import cgi
import os

form = cgi.FieldStorage()

print("Content-Type: text/html")
print()

print("""<!DOCTYPE html>
<html>
<head>
    <title>File Upload Handler</title>
</head>
<body>
    <h1>File Upload Result</h1>""")

if "file" in form:
    fileitem = form["file"]
    
    if fileitem.filename:
        # Get file info
        fn = os.path.basename(fileitem.filename)
        
        # In real implementation, save the file
        print(f"<p><strong>File uploaded:</strong> {fn}</p>")
        print(f"<p><strong>Content Type:</strong> {fileitem.type}</p>")
        
        # Show first 100 bytes of file
        data = fileitem.file.read(100)
        if data:
            print(f"<p><strong>First 100 bytes:</strong></p>")
            print(f"<pre>{data[:100]}</pre>")
    else:
        print("<p>No file was uploaded.</p>")
else:
    print("<p>No file field in form.</p>")

print("""
    <hr>
    <form action="upload.py" method="POST" enctype="multipart/form-data">
        <input type="file" name="file" required>
        <button type="submit">Upload Another File</button>
    </form>
    <a href="/">Back to Home</a>
</body>
</html>""")