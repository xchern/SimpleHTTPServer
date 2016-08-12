#pragma once

extern char mod_list[];

extern bool mod_existP(const char * name);
extern bool mod_serve(const char * name, const char * param, char * target);
extern void mod_doUnload(const char * name);
extern void mod_doLoad(const char * path);

extern void mod_status(void);
extern void mod_candidates(const char *);
