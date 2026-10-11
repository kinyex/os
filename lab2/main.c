#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <unistd.h>
#include <pthread.h>

#include <sys/time.h>

typedef struct {
	size_t number;
	size_t size;
	size_t n_threads;
	char* filename;
    int* arr;
    bool quiet;
    int min;
    int max;
} ThreadArgs;

static void* work(void* _args)
{
	ThreadArgs* args = (ThreadArgs*)_args;

    int min = INT_MAX;
    int max = INT_MIN;
    size_t len = args->size * sizeof(int);
    size_t x = args->number * (args->size / args->n_threads);
    size_t y = (args->number == args->n_threads - 1) ? args->size : (args->number + 1) * (args->size / args->n_threads);
    for (size_t i = x; i < y; i++) {
        if (args->arr[i] < min) {
            min = args->arr[i];
        }
        if (args->arr[i] > max) {
            max = args->arr[i];
        }
    }

	if (!args->quiet) {
        printf("Thread #%ld says min-max is: %d-%d\n", args->number, min, max);
    }

    args->min = min;
    args->max = max;

	return NULL;
}

int main(int argc, char** argv)
{
	if (argc <= 3) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg), "usage: %s filename array-size number-of-threads [-q]\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}
    bool quiet = argc > 4 && argv[4][0] == '-' && argv[4][1] == 'q';

	size_t n_threads = atoi(argv[3]);

	pthread_t *threads = malloc(n_threads * sizeof(pthread_t));
	ThreadArgs *thread_args = malloc(n_threads * sizeof(ThreadArgs));

    ssize_t bytes;
    size_t len = atoi(argv[2]) * sizeof(int);
    int* arr = (int*)malloc(len);
    int32_t file = open(argv[1], O_RDONLY);
	bytes = read(file, arr, len);
    close(file);


    struct timeval start, end;
    gettimeofday(&start, NULL);

	for (size_t i = 0; i < n_threads; ++i) {
		thread_args[i] = (ThreadArgs){
			.number = i,
			.size = atoi(argv[2]),
			.n_threads = n_threads,
			.filename = argv[1],
            .arr = arr,
            .quiet = quiet,
		};

		pthread_create(&threads[i], NULL, work, &thread_args[i]);
	}

	for (size_t i = 0; i < n_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

    int min = INT_MAX;
    int max = INT_MIN;
    for (size_t i = 0; i < n_threads; i++) {
        if (thread_args[i].min < min) {
            min = thread_args[i].min;
        }
        if (thread_args[i].max > max) {
            max = thread_args[i].max;
        }
    }
    gettimeofday(&end, NULL);
    double time_taken = end.tv_sec + end.tv_usec / 1e6 - start.tv_sec - start.tv_usec / 1e6;

    printf("Final min-max: %d-%d\n", min, max);
    printf("Time: %f\n", time_taken);

    free(arr);
	free(thread_args);
	free(threads);
}