cc client.c -o client 2> /dev/null && cc server.c -o server  2> /dev/null && ./client test1 test2 || echo "error"
