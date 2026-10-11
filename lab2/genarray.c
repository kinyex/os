#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>

#include <unistd.h>
#include <pthread.h>

int main(int argc, char **argv)
{
	if (argc <= 2) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg), "usage: %s filename array-size\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}
    
    srand(time(NULL));
    int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    size_t len = atoi(argv[2]) * sizeof(int);
    int* buf = (int*)malloc(len);
    
    int i = 0;
    for (; i < atoi(argv[2]); i++) {
        int value = rand();
        buf[i] = value;
    }

    write(file, buf, len);
    close(file);

    printf("An array of %d items has been generated\n", atoi(argv[2]));
}