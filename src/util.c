#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void die(const char *msg) {
	if (msg[0] && msg[strlen(msg) - 1] == ':') {
		fprintf(stderr, "%s\n", msg);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}
void malloc_check(void *ptr) {
	if (ptr == NULL) {
		perror("ERROR allocating memory\n");
		exit(2);
	}
}
void *ecalloc(size_t nmemb, size_t size) {
	void *p;

	if (!(p = calloc(nmemb, size))) die("calloc:");
	return p;
}
