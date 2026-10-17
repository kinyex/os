file="array"
len=$1

cc main.c -pthread -o main 2> /dev/null
cc genarray.c -o genarray 2> /dev/null

./genarray $file $len

echo
echo "1 thread:"
./main $file $len 1

echo
echo "2 threads:"
./main $file $len 2

echo
echo "4 threads:"
./main $file $len 4

echo
echo "6 threads:"
./main $file $len 6

echo
echo "12 threads:"
./main $file $len 12

echo
echo "32 threads:"
./main $file $len 32

echo
echo "1024 threads:"
./main $file $len 1024

echo
echo "8192 threads:"
./main $file $len 8192
