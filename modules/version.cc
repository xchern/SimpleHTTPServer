#include <string.h>
#include <stdio.h>

extern "C" const char * getName(void) { return "version";}
extern "C" void load(void);
extern "C" void unload(void);
extern "C" void serve(const char * param, char * target);

void load(void) { }

void unload(void) { }

void serve(const char * param, char * target) {
	char strversion[2048];
	FILE *p;
	int count;
	strcpy(target, "<html><head><title>version</title></head><body><pre>\n");

	p = popen("uname -a", "r");
	if(!p) {
		fprintf(stderr, "Error opening pipe.\n");
	}
	count = fread(strversion, sizeof(char), sizeof(strversion)/sizeof(char), p);
	if (pclose(p) == -1) {
		fprintf(stderr," Error closing pipe!\n");
	}
	strversion[count] = '\0';
	strcat(target, strversion);

	p = popen("cat /etc/*release", "r");
	if(!p) {
		fprintf(stderr, "Error opening pipe.\n");
	}
	count = fread(strversion, sizeof(char), sizeof(strversion)/sizeof(char), p);
	if (pclose(p) == -1) {
		fprintf(stderr," Error closing pipe!\n");
	}
	strversion[count] = '\0';
	strcat(target, strversion);
	strcat(target, "</pre></body></html>");
}
