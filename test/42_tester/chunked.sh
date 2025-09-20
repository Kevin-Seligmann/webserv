#!/bin/bash
FILE="post-data"
CHUNK_SIZE=10

# Read the file in chunks and pipe to curl
# Each chunk is sent as soon as it is read
split -b $CHUNK_SIZE "$FILE" chunk_

# Concatenate the chunks and pipe to curl
cat chunk_* | curl -v -K curl_chunked.txt

# Clean up temporary chunks
rm chunk_*
