#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/fcntl.h>
#include <wait.h>
#include <semaphore.h>
#include <sys/mman.h>

#define SHM_SIZE 4096

const char SHM_NAME[] = "example-shm";
const char SEM_NAME[] = "example-sem";

static char SERVER_PROGRAM_NAME[] = "server";

int main(int argc, char **argv) {
	if (argc <= 2) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg), "usage: %s filename1 filename2\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}

	char progpath[1024];
	ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
	if (len == -1) {
		const char msg[] = "error: failed to read full program path\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

	while (progpath[len] != '/') {
		--len;
	}

	progpath[len] = '\0';

	int shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (shm == -1) {
		const char msg[] = "error: failed to create SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}

	if (ftruncate(shm, SHM_SIZE) == -1) {
		const char msg[] = "error: failed to resize SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}

	char *shm_buf = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm, 0);
	if (shm_buf == MAP_FAILED) {
		const char msg[] = "error: failed to map SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}

	sem_unlink(SEM_NAME);
	sem_t *sem = sem_open(SEM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600, 1);
	if (sem == SEM_FAILED) {
		const char msg[] = "error: failed to create semaphore\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		_exit(EXIT_FAILURE);
	}

	const pid_t child = fork();

	switch (child) {
	case -1: {
		const char msg[] = "error: failed to spawn new process\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	} break;
	case 0: {
		pid_t pid = getpid();

		char msg[64];
		const int32_t length = snprintf(msg, sizeof(msg), "%d: I'm a child\n", pid);
		write(STDOUT_FILENO, msg, length);

		char path[1024];
		snprintf(path, sizeof(path) - 1, "%s/%s", progpath, SERVER_PROGRAM_NAME);

		char *const args[] = {SERVER_PROGRAM_NAME, argv[1], "1", NULL};

		int32_t status = execv(path, args);

		if (status == -1) {
			const char msg[] = "error: failed to exec into new exectuable image\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}
	} break;
	default: {

		const pid_t childchild = fork();

		switch (childchild) {
		case -1: {
			const char msg[] = "error: failed to spawn new process\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		} break;
		case 0: {
			pid_t pid = getpid();

			char msg[64];
			const int32_t length = snprintf(msg, sizeof(msg),
				"%d: I'm a child\n", pid);
			write(STDOUT_FILENO, msg, length);

			char path[1024];
			snprintf(path, sizeof(path) - 1, "%s/%s", progpath, SERVER_PROGRAM_NAME);

			char *const args[] = {SERVER_PROGRAM_NAME, argv[2], "2", NULL};

			int32_t status = execv(path, args);

			if (status == -1) {
				const char msg[] = "error: failed to exec into new exectuable image\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
		} break;
		default: {
			pid_t pid = getpid();

			char msg[128];
			const int32_t length = snprintf(msg, sizeof(msg),
				"%d: I'm a parent, my children has PID %d and PID %d\n", pid, child, childchild);
			write(STDOUT_FILENO, msg, length);

			char buf[SHM_SIZE];
			ssize_t bytes;

			while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
				sem_wait(sem);

				uint32_t* shm_length = (uint32_t*)shm_buf;
				char* id = shm_buf + sizeof(uint32_t);
				char* text = shm_buf + sizeof(uint32_t) + sizeof(char);

				if (bytes < 0) {
					*shm_length = UINT32_MAX;
					sem_post(sem);
					const char msg[] = "error: failed to read from stdin\n";
					write(STDERR_FILENO, msg, sizeof(msg));
					exit(EXIT_FAILURE);
				}
				else if (buf[0] == '\n') {
					sem_post(sem);
					break;
				}

				*shm_length = bytes;
				memcpy(id, bytes > 11 ? "2" : "1", sizeof(char));
				memcpy(text, buf, bytes);

				sem_post(sem);
			}
			*(uint32_t*)shm_buf = UINT32_MAX;

			wait(NULL);
			wait(NULL);
			sem_unlink(SEM_NAME);
			sem_close(sem);
			munmap(shm_buf, SHM_SIZE);
			shm_unlink(SHM_NAME);
			close(shm);
		} break;
		}
	} break;
	}
}
