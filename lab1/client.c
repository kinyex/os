#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static char SERVER_PROGRAM_NAME[] = "server";

int main(int argc, char **argv) {
	if (argc <= 2) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg) - 1, "usage: %s filename1 filename2\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}

	char progpath[1024];
	{
		ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
		if (len == -1) {
			const char msg[] = "error: failed to read full program path\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		while (progpath[len] != '/')
			--len;

		progpath[len] = '\0';
	}

	int client_to_server1[2];
	if (pipe(client_to_server1) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

	int server_to_client1[2];
	if (pipe(server_to_client1) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
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
		const int32_t length = snprintf(msg, sizeof(msg),
			"%d: I'm a child\n", pid);
		write(STDOUT_FILENO, msg, length);

		close(client_to_server1[1]);
		close(server_to_client1[0]);

		dup2(client_to_server1[0], STDIN_FILENO);
		close(client_to_server1[0]);

		dup2(server_to_client1[1], STDOUT_FILENO);
		close(server_to_client1[1]);

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
		int client_to_server2[2];
		if (pipe(client_to_server2) == -1) {
			const char msg[] = "error: failed to create pipe\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		int server_to_client2[2];
		if (pipe(server_to_client2) == -1) {
			const char msg[] = "error: failed to create pipe\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

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

			close(client_to_server2[1]);
			close(server_to_client2[0]);

			dup2(client_to_server2[0], STDIN_FILENO);
			close(client_to_server2[0]);

			dup2(server_to_client2[1], STDOUT_FILENO);
			close(server_to_client2[1]);

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

			close(client_to_server1[0]);
			close(server_to_client1[1]);
			close(client_to_server2[0]);
			close(server_to_client2[1]);

			char buf[4096];
			ssize_t bytes;

			while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
				if (bytes < 0) {
					const char msg[] = "error: failed to read from stdin\n";
					write(STDERR_FILENO, msg, sizeof(msg));
					exit(EXIT_FAILURE);
				}
				if (buf[0] == '\n') {
					break;
				}
				if (bytes > 11) {
					write(client_to_server2[1], buf, bytes);

					bytes = read(server_to_client2[0], buf, sizeof(buf));
					write(STDOUT_FILENO, buf, bytes);
				}
				else {
					write(client_to_server1[1], buf, bytes);

					bytes = read(server_to_client1[0], buf, sizeof(buf));
					write(STDOUT_FILENO, buf, bytes);
				}
			}

			close(client_to_server1[1]);
			close(server_to_client1[0]);
			close(client_to_server2[1]);
			close(server_to_client2[0]);

			wait(NULL);
		} break;
		}
	} break;
	}
}
