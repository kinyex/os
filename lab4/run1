#!/usr/bin/env bash
f=0
cc -fPIC -shared library_1.c -o library_1.so 2> /dev/null &&
cc program1.c -L. -lrary_1 -o bin1 2> /dev/null ||
f=1
if [ $f -eq 1 ]; then
    echo "error"
else
    export LD_LIBRARY_PATH=.
    ./bin1
fi