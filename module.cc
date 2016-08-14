#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>

#include <string>
#include <map>

#include "module.h"

struct ModItem {
	void * dlp;
	void (*load)(void);
	void (*unload)(void);
	void (*serve)(const char *, char *);
};

pthread_rwlock_t list_rw_lock = PTHREAD_RWLOCK_INITIALIZER;
static std::map <std::string, struct ModItem> modules;

char mod_list[4096];

void mod_list_update(void) {
	mod_list[0] = '\0';
	std::map<std::string, struct ModItem>::iterator it = modules.begin();
	for (; it != modules.end(); ++it) {
		strcat(mod_list, it->first.c_str());
		strcat(mod_list, "\n");
	}
}

void mod_status(void){
	fprintf(stderr, "All running modules:\n");
	fprintf(stderr, mod_list);
}

void mod_candidates(const char * path){
	DIR * dir;
	struct dirent *ent;
	if ((dir = opendir(path)) != NULL) {
		fprintf(stderr, "module candidates:\n");
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] != '.')
				fprintf(stderr, "\t%s", ent->d_name);
		}
		fprintf(stderr, "\n");
		closedir(dir);
	} else {
		fprintf(stderr, "fail open module directory!\n");
	}
}

bool mod_serve(const char * name, const char * param, char * target) {
	std::string modname(name);
	bool result = false;
	int rl = pthread_rwlock_rdlock(&list_rw_lock);
	if (modules.count(modname) > 0) {
		modules[modname].serve(param, target);
		result = true;
	}
	pthread_rwlock_unlock(&list_rw_lock);
	return result;
}

void mod_doUnload(const char * name) {
	fprintf(stderr, "try to unload module \'%s\'\n", name);
	std::string modname(name);
	// check if module exists
	if (!modules.count(modname) > 0) {
		fprintf(stderr, "no module named \'%s\'\n", name);
		return;
	}

	// unregister module
	fprintf(stderr, "unregistering module...");
	struct ModItem mod = modules[modname];
	modules.erase(modname);
	mod_list_update();
	fprintf(stderr, "done.\n");

	// unload module
	fprintf(stderr, "unloading module...");
	mod.unload();
	fprintf(stderr, "done.\n");

	// close shared library
	fprintf(stderr, "closing shared library...");
	if (dlclose(mod.dlp)) {
		fprintf(stderr, "fail\n");
		char * errormsg = dlerror();
		if (errormsg)
			fprintf(stderr, errormsg);
	} else {
		fprintf(stderr, "done.\n");
		fprintf(stderr, "SUCCESS!\n");
	}
}

void mod_doLoad(const char * path) {
	struct ModItem newmod;
	std::string modname;
	char * errormsg;
	fprintf(stderr, "try to open library from file %s...", path);
	// try load shared lib
	newmod.dlp = dlopen(path, RTLD_LAZY);
	if (newmod.dlp) {
		fprintf(stderr, "done.\n");
		// try get module information
		fprintf(stderr, "try to getmodule information.\n");
		const char * (* getName)(void) = (const char * (*)()) dlsym(newmod.dlp, "getName");
		if (!getName) goto bad_mod;
		modname = getName();
		fprintf(stderr, "module name: %s\n", modname.c_str());

		if (modules.count(modname) > 0) { // abort if name exists
			fprintf(stderr, "module with same name having been loaded\n");
			goto close_lib;
		}

		// try get interface
		newmod.load = (void (*)()) dlsym(newmod.dlp, "load");
		if (!newmod.load) goto bad_mod;
		newmod.unload = (void (*)()) dlsym(newmod.dlp, "unload");
		if (!newmod.unload) goto bad_mod;
		newmod.serve = (void (*)(const char *, char *)) dlsym(newmod.dlp, "serve");
		if (!newmod.serve) goto bad_mod;

		// load module
		fprintf(stderr, "loading module...");
		newmod.load();
		fprintf(stderr, "done.\n");

		// register module
		fprintf(stderr, "registering module...");
		modules[modname] = newmod;
		mod_list_update();
		fprintf(stderr, "done.\n");

		fprintf(stderr, "SUCCESS!\n");
		return;
bad_mod:
		fprintf(stderr, "BAD module.\n");
		errormsg = dlerror();
		if (errormsg)
			fprintf(stderr, errormsg);
close_lib:
		if (dlclose(newmod.dlp)) {
			fprintf(stderr, "fail closing shared library.\n");
			errormsg = dlerror();
			if (errormsg)
				fprintf(stderr, errormsg);
		}
	} else {
		fprintf(stderr, "fail.\n");
		errormsg = dlerror();
		if (errormsg)
			fprintf(stderr, errormsg);
	}
}
