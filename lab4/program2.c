#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <dlfcn.h>

#include "library_run.h"

static prime_count_func *prime_count;
static pi_func *pi;

static int func3_impl_stub(int a, int b) {
	(void)a;
	(void)b;
	return 0;
}

static float func5_impl_stub(int k) {
	(void)k;
	return 0.f;
}

void* load_library(char* lib) {
	void* library = dlopen(lib, RTLD_LOCAL | RTLD_NOW);

	if (library == NULL) {
		const char msg[] = "error: failed to open library\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(1);
	}

	prime_count = dlsym(library, "prime_count");
	if (prime_count == NULL) {
		const char msg[] = "warning: failed to find prime_count function implementation\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		prime_count = func3_impl_stub;
	}

	pi = dlsym(library, "pi");
	if (pi == NULL) {
		const char msg[] = "warning: failed to find pi function implementation\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		pi = func5_impl_stub;
	}

	return library;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("usage: %s library_1 library_2\n", argv[0]);
		exit(1);
	}

	int current_library = 1;
	load_library(argv[current_library]);

	void* library;

	int bytes;
	char buf[1024];
	while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
		if (buf[0] == '0') {
			current_library = 3 - current_library;
			if (library) {
				dlclose(library);
			}
			library = load_library(argv[current_library]);
			continue;
		}
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

	if (library) {
		dlclose(library);
	}

	return 0;
}