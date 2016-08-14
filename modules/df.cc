#include <string.h>
#include <stdio.h>

extern "C" const char * getName(void) { return "df";}
extern "C" void load(void);
extern "C" void unload(void);
extern "C" void serve(const char * param, char * target);

void load(void) { }

void unload(void) { }

void serve(const char * param, char * target) {
	char strdf[2048];
	FILE *p;
	strcpy(target, "<html><head><title>df</title></head><body><pre>\n");
	// df -l | sed -e 's/\s\+/<\/td><td>/g' -e 's/$/<\/td><\/tr>/' -e 's/^/<tr><td>/'
	// p = popen("df -l | sed -e 's/\\s\\+/<\\/td><td>/g' -e 's/$/<\\/td><\\/tr>/' -e 's/^/<tr><td>/'", "r");
	p = popen("df -l", "r");
	if(!p) {
		fprintf(stderr, "Error opening pipe.\n");
	}
	int count = fread(strdf, sizeof(char), sizeof(strdf)/sizeof(char), p);
	strdf[count] = '\0';
	if (pclose(p) == -1) {
		fprintf(stderr," Error closing pipe!\n");
	}
	strcat(target, strdf);
	strcat(target, "</pre></body></html>");
}
