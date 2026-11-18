#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <dlfcn.h>

#include "library_comp.h"

int main(int argc, char **argv) {
	(void)argc;

	int bytes;
	char buf[1024];
	while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
		if (buf[0] != '1' && buf[0] != '2') {
			printf("error: unexpected command\n");
			exit(1);
		}
		char buf1[1024];
		char buf2[1024];
		size_t i = 2;
		size_t offset = i;
		for (; i < bytes; i++) {
			if (buf[i] == '\0' || buf[i] == '\n' || buf[i] == ' ') {
				buf1[i - offset] = '\0';
				break;
			}
			buf1[i - offset] = buf[i];
		}
		i++;
		offset = i;
		for (; i < bytes; i++) {
			if (buf[i] == '\0' || buf[i] == '\n' || buf[i] == ' ') {
				buf2[i - offset] = '\0';
				break;
			}
			buf2[i - offset] = buf[i];
		}
		if (buf[0] == '1') {
			printf("prime count: %d\n", prime_count(atoi(buf1), atoi(buf2)));
		}
		else {
			printf("pi: %f\n", pi(atoi(buf1)));
		}
	}

	return 0;
}