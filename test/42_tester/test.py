import socket

host = "localhost"
port = 8081
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))

headers = (
    "POST /directory/youpi.bla HTTP/1.1\r\n"
    f"Host: {host}:{port}\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
)
s.sendall(headers.encode())

chunk1 = "5\r\nHello\r\n"
s.sendall(chunk1.encode())

chunk2 = "6\r\nWorld!\r\n"
s.sendall(chunk2.encode())

end_chunk = "0\r\n\r\n"
s.sendall(end_chunk.encode())

response_parts = []
while True:
    data = s.recv(4096)
    if not data:
        break
    response_parts.append(data)

response = b"".join(response_parts)
print("Response:\n", response.decode(errors="ignore"))

s.close()