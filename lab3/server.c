#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_SIZE 4096

const char SHM_NAME[] = "example-shm";
const char SEM_NAME[] = "example-sem";

int main(int argc, char **argv) {
	if (argc <= 2) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg), "usage: %s filename number\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}

	int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (file == -1) {
		const char msg[] = "error: failed to open requested file\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

	int shm = shm_open(SHM_NAME, O_RDWR, 0);
	if (shm == -1) {
		const char msg[] = "error: failed to open SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}

	char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
	if (shm_buf == MAP_FAILED) {
		const char msg[] = "error: failed to map SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}

	sem_t *sem = sem_open(SEM_NAME, O_RDWR);
	if (sem == SEM_FAILED) {
		const char msg[] = "error: failed to open semaphore\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}

	char buf[SHM_SIZE];
	ssize_t len;

	bool running = true;
	while (running) {
		sem_wait(sem);

		uint32_t* shm_length = (uint32_t*)shm_buf;
		char* id = shm_buf + sizeof(uint32_t);
		char* text = shm_buf + sizeof(uint32_t) + sizeof(char);

		if (*shm_length == UINT32_MAX) {
			running = false;
		} else if (*shm_length > 0) {
			if (id[0] != argv[2][0]) {
				sem_post(sem);
				continue;
			}

			len = 0;
			for (uint32_t i = 0; i < *shm_length; ++i) {
				char ch = text[i];
				char lch = tolower(text[i]);
				if (lch == 'a' || lch == 'o' || lch == 'u' || lch == 'i' || lch == 'e' || lch == 'y') {
					continue;
				}
				buf[len] = ch;
				len++;
			}

			int32_t written = write(file, buf, len);
			if (written != len) {
				const char msg[] = "error: failed to write to file\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
			
			char msg[64];
			const int32_t length = snprintf(msg, sizeof(msg), "Server%s received: ", argv[2]);
			write(STDERR_FILENO, msg, length);

			written = write(STDOUT_FILENO, buf, len);
			if (written != len) {
				const char msg[] = "error: failed to echo\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}

			*shm_length = 0;
		}

		sem_post(sem);
	}

	sem_close(sem);
	munmap(shm_buf, SHM_SIZE);
	close(shm);
	close(file);
}
