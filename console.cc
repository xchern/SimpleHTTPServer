#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "module.h"

#include "console.h"

static char buff[1024];

static char mod_prefix[512];

static char mod_path[1024];

const char help_items[] = "\
help	print this list\n\
status	show status\n\
cand	show candidate modules avaliable for loading\n\
load	load module\n\
unload	unload module\n\
";

void console(void) {
	char * mod_name;
	{
		char szTmp[64];
		sprintf(szTmp, "/proc/%d/exe", getpid());
		int bytes = readlink(szTmp, mod_prefix, sizeof(mod_prefix));
		if (bytes < sizeof(mod_prefix))
			bytes = sizeof(mod_prefix) - 1;
		if (bytes < 0)
			puts("unable to get current work directory");
		while (mod_prefix[--bytes] != '/');
		bytes++;
		strcpy(mod_prefix + bytes, "modules/");
		strcpy(mod_path, mod_prefix);
		mod_name = mod_path + strlen(mod_path);
	}
	printf("module probing directory: %s\n", mod_prefix);
	for (;;) {
		// prompt
		printf("\n>>> ");
		// read
		int i = 0;
		char c; 
		while ((c = getchar()) != '\n') {
			buff[i] = c;
			// prevent overflow
			if (i < sizeof(buff)/sizeof(char)) i++;
		}
		buff[i] = '\0';
		// parse & do
		if (!strcmp(buff, "quit")) { break; }
		if (!strcmp(buff, "help")) { puts(help_items); continue; }
		if (!strcmp(buff, "status")) { mod_status(); continue; }
		if (!strcmp(buff, "cand")) { mod_candidates(mod_prefix); continue; }

		// split two params
		i = 0;
		while ((c = buff[i]) != '\0') {
			if (c == ' ') { buff[i] = '\0'; i++; break; }
			i++;
		}
		const char * name = buff + i;

		if (!strcmp(buff, "load")) { strcpy(mod_name, name); mod_doLoad(mod_path); continue; }
		if (!strcmp(buff, "unload")) { mod_doUnload(name); continue; }
		// else bad command
		puts("Bad Command!");
	}
	return;
}
