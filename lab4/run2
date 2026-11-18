#!/usr/bin/env bash
f=0
cc -fPIC -shared library_1.c -o library_1.so 2> /dev/null &&
cc -fPIC -shared library_2.c -o library_2.so 2> /dev/null &&
cc -ldl program2.c -o bin2 2> /dev/null ||
f=1
if [ $f -eq 1 ]; then
    echo "error"
else
    ./bin2 ./library_1.so ./library_2.so
fi