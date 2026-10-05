#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv) {
	char buf[4096];
	char buf1[4096];
	ssize_t bytes;
	ssize_t len;

	pid_t pid = getpid();

	int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (file == -1) {
		const char msg[] = "error: failed to open requested file\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}
	
	while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
		if (bytes < 0) {
			const char msg[] = "error: failed to read from stdin\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		len = 0;
		for (uint32_t i = 0; i < bytes; ++i) {
			char ch = buf[i];
			char lch = tolower(buf[i]);
			if (lch == 'a' || lch == 'o' || lch == 'u' || lch == 'i' || lch == 'e' || lch == 'y') continue;
			buf1[len] = ch;
			len++;
		}

		{
			int32_t written = write(file, buf1, len);
			if (written != len) {
				const char msg[] = "error: failed to write to file\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}

			{
				char msg[64];
				const int32_t length = snprintf(msg, sizeof(msg), "Server%s received: ", argv[2]);
				write(STDERR_FILENO, msg, length);
			}

			written = write(STDOUT_FILENO, buf1, len);
			if (written != len) {
				const char msg[] = "error: failed to echo\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
		}
	}

	close(file);
}
