#include <string.h>
#include <stdio.h>

extern "C" const char * getName(void) { return "time";}
extern "C" void load(void);
extern "C" void unload(void);
extern "C" void serve(const char * param, char * target);

void load(void) { }

void unload(void) { }

void serve(const char * param, char * target) {
	char strTime[256];
	FILE *p;
	strcpy(target, "<html><head><title>time</title></head><body><pre>\n");
	p = popen("date", "r");
	if(!p) {
		fprintf(stderr, "Error opening pipe.\n");
	}
	int count = fread(strTime, sizeof(char), sizeof(strTime)/sizeof(char), p);
	strTime[count] = '\0';
	if (pclose(p) == -1) {
		fprintf(stderr," Error closing pipe!\n");
	}
	strcat(target, strTime);
	strcat(target, "</pre></body></html>");
}
