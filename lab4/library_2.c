#include <unistd.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT int prime_count(int a, int b) {
	write(STDOUT_FILENO, "(2)", 4);
	bool* arr = (bool*)malloc((b - 1) * sizeof(bool));
	for (int i = 2; i <= b; i++) {
		arr[i - 2] = true;
	}

	int s = 0;

	for (int i = 2; i <= b; i++) {
		if (arr[i - 2] == false) {
			continue;
		}
		for (int j = i; j <= b; j += i) {
			arr[j - 2] = false;
		}
		if (i >= a) {
			s++;
		}
	}

	return s;
}

EXPORT float pi(int k) {
	write(STDOUT_FILENO, "(2)", 4);
	float res = 1.f;
	for (int i = 1; i <= k; i++) {
		res *= (4.f * i * i) / (4.f * i * i - 1.f);
	}
	return res * 2;
}

