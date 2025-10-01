#!/bin/bash

echo "Setting up test environment..."

# Create directory structure
mkdir -p www/{errors,static/{css,js,img,files},uploads,cgi-bin,php}
mkdir -p api/{api-errors,v1,admin}
mkdir -p files/{documents,images,private}
mkdir -p logs
mkdir -p tests

# Create empty files for directory structure
touch www/uploads/.keep
touch logs/.keep

# Make CGI scripts executable
chmod +x www/cgi-bin/*.py
chmod +x www/php/*.php

echo "Test environment ready!"