#!/usr/bin/env python3
import os
import sys
from email.parser import BytesParser
from email.policy import default

UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "../uploads_dest")
MAX_SIZE = 10 * 1024 * 1024

print("Content-Type: text/html; charset=utf-8\r")
print("\r")

print("""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>File Upload</title>
    <style>
        body { font-family: Arial; max-width: 800px; margin: 50px auto; padding: 20px; }
        .success { color: green; }
        .error { color: red; }
        .upload-form { background: #f9f9f9; padding: 20px; border-radius: 8px; }
        pre { background: #f4f4f4; padding: 10px; overflow-x: auto; }
        button { background: #007bff; color: white; padding: 10px 20px; 
                 border: none; border-radius: 4px; cursor: pointer; }
        button:hover { background: #0056b3; }
    </style>
</head>
<body>""")

method = os.environ.get('REQUEST_METHOD', 'GET')

# ========== GET: Mostrar formulario ==========
if method == 'GET':
    print("""
    <h1>File Upload</h1>
    <div class="upload-form">
        <form method="POST" enctype="multipart/form-data">
            <p><input type="file" name="file" required></p>
            <button type="submit">Upload File</button>
        </form>
    </div>
    <hr>
    <p><a href="/">← Back to Home</a></p>
    """)

# ========== POST: Procesar upload ==========
elif method == 'POST':
    print("<h1>Upload Result</h1>")
    
    try:
        if not os.path.exists(UPLOAD_DIR):
            os.makedirs(UPLOAD_DIR, 0o755)
        
        content_type = os.environ.get('CONTENT_TYPE', '')
        
        if 'multipart/form-data' not in content_type:
            print(f"<p class='error'>Error: Expected multipart/form-data</p>")
            print(f"<p>Received: {content_type}</p>")
            raise SystemExit
        
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        
        if content_length == 0:
            print("<p class='error'>Error: No file data received</p>")
            raise SystemExit
        
        if content_length > MAX_SIZE:
            print(f"<p class='error'>Error: File too large ({content_length:,} bytes)</p>")
            print(f"<p>Maximum: {MAX_SIZE:,} bytes</p>")
            raise SystemExit
        
        body = sys.stdin.buffer.read(content_length)
        
        http_message = b'Content-Type: ' + content_type.encode('latin-1') + b'\r\n\r\n' + body
        msg = BytesParser(policy=default).parsebytes(http_message)
        
        uploaded_files = []
        
        for part in msg.walk():
            if part.get_content_maintype() == 'multipart':
                continue
            
            disp = part.get('Content-Disposition', '')
            if not disp or 'filename=' not in disp:
                continue
            
            filename = part.get_filename()
            if not filename:
                continue
            
            filename = os.path.basename(filename)
            if not filename or filename.startswith('.'):
                continue
            
            file_data = part.get_payload(decode=True)
            
            filepath = os.path.join(UPLOAD_DIR, filename)
            
            counter = 1
            base, ext = os.path.splitext(filename)
            while os.path.exists(filepath):
                filename = f"{base}_{counter}{ext}"
                filepath = os.path.join(UPLOAD_DIR, filename)
                counter += 1
            
            with open(filepath, 'wb') as f:
                f.write(file_data)
            
            uploaded_files.append({
                'name': filename,
                'path': filepath,
                'size': len(file_data),
                'type': part.get_content_type()
            })
        
        if uploaded_files:
            print("<h2 class='success'>✓ Upload Successful</h2>")
            for f in uploaded_files:
                print(f"<p><strong>File:</strong> {f['name']}</p>")
                print(f"<p><strong>Saved to:</strong> <code>{f['path']}</code></p>")
                print(f"<p><strong>Size:</strong> {f['size']:,} bytes</p>")
                print(f"<p><strong>Type:</strong> {f['type']}</p>")
                
                # Preview para archivos de texto
                if f['type'].startswith('text/'):
                    try:
                        with open(f['path'], 'r') as preview:
                            content = preview.read(200)
                            print("<p><strong>Preview:</strong></p>")
                            print(f"<pre>{content}...</pre>")
                    except:
                        pass
                
                print("<hr>")
        else:
            print("<p class='error'>No files found in upload</p>")
    
    except Exception as e:
        print(f"<p class='error'>Error: {e}</p>")
        print("<h3>Debug Info:</h3><pre>")
        print(f"CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', 'NOT SET')}")
        print(f"CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', 'NOT SET')}")
        print(f"UPLOAD_DIR: {UPLOAD_DIR}")
        print("</pre>")
    
    print("""
    <hr>
    <p><a href="/cgi-bin/upload.py">← Upload Another File</a></p>
    <p><a href="/">← Back to Home</a></p>
    """)

print("</body></html>")