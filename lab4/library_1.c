#include <unistd.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT int prime_count(int a, int b) {
	write(STDOUT_FILENO, "(1)", 4);
	int s = 0;
	for (int t = a; t <= b; t++) {
		if (t < 2) {
			continue;
		}
		bool f = true;
		for (int i = 2; i < t; i++) {
			if (t % i == 0) {
				f = false;
				break;
			}
		}
		if (f) {
			s++;
		}
	}
	return s;
}

EXPORT float pi(int k) {
	write(STDOUT_FILENO, "(1)", 4);
	float res = 0;
	for (int i = 0; i < k; i++) {
		res += (1.f / (2 * i + 1)) * (1 - i % 2 * 2);
	}
	return res * 4;
}
