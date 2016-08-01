#include <stdio.h>
#include <string.h>

#include "module.h"

#include "console.h"

static char buff[1024];

const char help_items[] = "\
help	print this list\n\
status	show status\n\
cand	show candidate modules avaliable for loading\n\
load	load module\n\
unload	unload module\n\
";

void console(void) {
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
		if (!strcmp(buff, "help")) { puts(help_items); continue; }
		if (!strcmp(buff, "status")) { mod_status(); continue; }
		if (!strcmp(buff, "cand")) { mod_candidates(); continue; }

		// split two params
		i = 0;
		while ((c = buff[i]) != '\0') {
			if (c == ' ') { buff[i] = '\0'; i++; break; }
			i++;
		}
		const char * path = buff + i;

		if (!strcmp(buff, "load")) { mod_doLoad(path); continue; }
		if (!strcmp(buff, "unload")) { mod_doUnload(path); continue; }
		// else bad command
		puts("Bad Command!");
	}
	return;
}
