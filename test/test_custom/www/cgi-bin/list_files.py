#!/usr/bin/env python3
import os
import json

UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "../uploads_dest")

print("Content-Type: application/json")
print()

files = []
try:
    if os.path.exists(UPLOAD_DIR):
        for filename in os.listdir(UPLOAD_DIR):
            filepath = os.path.join(UPLOAD_DIR, filename)
            if os.path.isfile(filepath) and filename != '.keep':
                file_stat = os.stat(filepath)
                files.append({
                    'name': filename,
                    'size': file_stat.st_size
                })
except Exception as e:
    pass

print(json.dumps({'files': files}))
