#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[]) {

	char * filename = NULL;

	if (argc > 1) {
		filename = argv[1];
	}

	FILE * in = stdin;
	if (filename) {
		in = fopen(filename, "r");
	}
	FILE * out = stdout;

	char c, last;
	int tabs = 0;
	c = 0;

	while ((c = fgetc(in)) != EOF) {
		fprintf(out, "%c", c);
		if (feof(in))
			break;
		if (c == '<') {
			tabs++;
		}
		if (c == '/') {
			if (last == '<') {
				tabs -= 2;
			}
		}
		if (c == '>') {
			fprintf(out, "\n");
			if (last == '/')
				tabs --;
			for (int i = 0; i < tabs; i++) {
				fprintf(out, "\t");
			}
		}
		last = c;
	}

	return 0;
}
