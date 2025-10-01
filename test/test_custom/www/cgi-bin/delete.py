#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import json
import urllib.parse

def get_uploads_dir():
    """Get the absolute path to uploads_dest directory"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    uploads_dir = os.path.join(script_dir, '..', 'uploads_dest')
    return os.path.normpath(uploads_dir)

def list_files():
    """List all files in uploads_dest directory"""
    uploads_dir = get_uploads_dir()
    
    try:
        files = []
        if os.path.exists(uploads_dir):
            for filename in os.listdir(uploads_dir):
                filepath = os.path.join(uploads_dir, filename)
                if os.path.isfile(filepath) and filename != '.keep':
                    file_stat = os.stat(filepath)
                    files.append({
                        'name': filename,
                        'size': file_stat.st_size,
                        'path': '/uploads_dest/' + filename
                    })
        return files
    except Exception as e:
        return []

def delete_file(filename):
    """Delete a specific file from uploads_dest"""
    uploads_dir = get_uploads_dir()
    
    # Security: prevent directory traversal
    if '..' in filename or '/' in filename or '\\' in filename:
        return False, "Invalid filename"
    
    filepath = os.path.join(uploads_dir, filename)
    
    try:
        if os.path.exists(filepath) and os.path.isfile(filepath):
            os.remove(filepath)
            return True, f"File '{filename}' deleted successfully"
        else:
            return False, "File not found"
    except Exception as e:
        return False, f"Error deleting file: {str(e)}"

def main():
    request_method = os.environ.get('REQUEST_METHOD', 'GET')
    
    if request_method == 'GET':
        # List files as HTML page
        files = list_files()
        
        print("Content-Type: text/html")
        print()
        print("<!DOCTYPE html>")
        print("<html>")
        print("<head>")
        print("    <meta charset='UTF-8'>")
        print("    <title>File Manager - DELETE Test</title>")
        print("    <style>")
        print("        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; }")
        print("        h1 { color: #333; }")
        print("        table { width: 100%; border-collapse: collapse; margin-top: 20px; }")
        print("        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }")
        print("        th { background-color: #4CAF50; color: white; }")
        print("        tr:hover { background-color: #f5f5f5; }")
        print("        button { background-color: #f44336; color: white; border: none; padding: 8px 16px; cursor: pointer; border-radius: 4px; }")
        print("        button:hover { background-color: #da190b; }")
        print("        .message { padding: 15px; margin: 20px 0; border-radius: 4px; }")
        print("        .success { background-color: #4CAF50; color: white; }")
        print("        .error { background-color: #f44336; color: white; }")
        print("        .empty { color: #666; font-style: italic; }")
        print("        .back-link { display: inline-block; margin-top: 20px; color: #4CAF50; text-decoration: none; }")
        print("        .back-link:hover { text-decoration: underline; }")
        print("    </style>")
        print("</head>")
        print("<body>")
        print("    <h1>File Manager - DELETE Test</h1>")
        print("    <p>Files in uploads_dest directory:</p>")
        
        if files:
            print("    <table>")
            print("        <tr>")
            print("            <th>Filename</th>")
            print("            <th>Size (bytes)</th>")
            print("            <th>Action</th>")
            print("        </tr>")
            
            for file in files:
                print(f"        <tr>")
                print(f"            <td>{file['name']}</td>")
                print(f"            <td>{file['size']}</td>")
                print(f"            <td>")
                print(f"                <button onclick='deleteFile(\"{file['name']}\")'>Delete</button>")
                print(f"            </td>")
                print(f"        </tr>")
            
            print("    </table>")
        else:
            print("    <p class='empty'>No files to display.</p>")
        
        print("    <a href='/' class='back-link'>← Back to Home</a>")
        print("    <script>")
        print("        function deleteFile(filename) {")
        print("            if (!confirm('Are you sure you want to delete ' + filename + '?')) return;")
        print("            ")
        print("            fetch('/cgi-bin/delete.py?file=' + encodeURIComponent(filename), {")
        print("                method: 'DELETE'")
        print("            })")
        print("            .then(response => response.json())")
        print("            .then(data => {")
        print("                if (data.success) {")
        print("                    alert(data.message);")
        print("                    location.reload();")
        print("                } else {")
        print("                    alert('Error: ' + data.message);")
        print("                }")
        print("            })")
        print("            .catch(error => {")
        print("                alert('Error: ' + error);")
        print("            });")
        print("        }")
        print("    </script>")
        print("</body>")
        print("</html>")
        
    elif request_method == 'DELETE':
        # Handle DELETE request
        query_string = os.environ.get('QUERY_STRING', '')
        filename = None
        
        # Parse query string to get filename
        if 'file=' in query_string:
            parts = query_string.split('&')
            for part in parts:
                if part.startswith('file='):
                    # URL decode properly
                    filename = urllib.parse.unquote(part[5:])
                    break
        
        if not filename:
            print("Content-Type: application/json")
            print()
            print(json.dumps({
                'success': False,
                'message': 'No filename specified'
            }))
            return
        
        success, message = delete_file(filename)
        
        print("Content-Type: application/json")
        print()
        print(json.dumps({
            'success': success,
            'message': message
        }))
    
    else:
        # Method not allowed
        print("Content-Type: application/json")
        print()
        print(json.dumps({
            'success': False,
            'message': f'Method {request_method} not allowed'
        }))

if __name__ == '__main__':
    main()
