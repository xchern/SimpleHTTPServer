#include <string.h>
#include <stdio.h>

extern "C" const char * getName(void) { return "ps";}
extern "C" void load(void);
extern "C" void unload(void);
extern "C" void serve(const char * param, char * target);

void load(void) { }

void unload(void) { }

void serve(const char * param, char * target) {
	char strps[23768];
	FILE *p;
	strcpy(target, "<html><head><title>ps</title></head><body><pre>\n");
	p = popen("pstree", "r");
	if(!p) {
		fprintf(stderr, "Error opening pipe.\n");
	}

	char * dest = strps;
	int free = sizeof(strps)/sizeof(char) - 1;
	int count = 0;
	while (count = fread(dest, sizeof(char), free, p)) {
		free -= count;
		dest += count;
		if (free == 0) break;
	}
	dest[0] = '\0';
	if (pclose(p) == -1) {
		fprintf(stderr," Error closing pipe!\n");
	}
	strcat(target, strps);
	strcat(target, "</pre></body></html>");
}
