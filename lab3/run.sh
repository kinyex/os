cc client.c -o client 2> /dev/null && cc server.c -o server  2> /dev/null && ./client file1 file2 || echo "error"
