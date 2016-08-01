#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>

#include <string>
#include <map>

#include "module.h"

struct ModItem {
	void * dlp;
	void (*load)(void);
	void (*unload)(void);
	const char * (*serve)(const char *);
};

pthread_rwlock_t list_rw_lock = PTHREAD_RWLOCK_INITIALIZER;
static std::map <std::string, struct ModItem> modules;

void mod_status(void){}; //TODO

void mod_candidates(void){}; //TODO

const char * mod_serve(const char * name, const char * param) {
	std::string modname(name);
	const char * result = NULL;
	int rl = pthread_rwlock_rdlock(&list_rw_lock);
	if (modules.count(modname) > 0)
		result = (modules[modname].serve(param));
	pthread_rwlock_unlock(&list_rw_lock);
	return result;
}

void mod_doUnload(const char * name) {
	puts("[unload]");
	printf("try to unload module \'%s\'\n", name);
	std::string modname(name);
	// check if module exists
	if (!modules.count(modname) > 0) {
		printf("no module named \'%s\'\n", name);
		return;
	}

	// unregister module
	printf("unregistering module...");
	struct ModItem mod = modules[modname];
	modules.erase(modname);
	puts("done.");

	// unload module
	printf("unloading module...");
	mod.unload();
	puts("done.");

	// close shared library
	printf("closing shared library...");
	if (dlclose(mod.dlp)) {
		puts("fail");
		char * errormsg = dlerror();
		if (errormsg)
			puts(errormsg);
	} else {
		puts("done.");
	}
}

void mod_doLoad(const char * path) {
	puts("[load]");
	struct ModItem newmod;
	std::string modname;
	char * errormsg;
	printf("try to open library from file %s...", path);
	// try load shared lib
	newmod.dlp = dlopen(path, RTLD_LAZY);
	if (newmod.dlp) {
		puts("done.");
		// try get module information
		puts("try to getmodule information.");
		const char * (* getName)(void) = (const char * (*)()) dlsym(newmod.dlp, "getName");
		if (!getName) goto bad_mod;
		modname = getName();
		printf("module name: %s\n", modname.c_str());

		if (modules.count(modname) > 0) { // abort if name exists
			puts("module with same name having been loaded");
			goto close_lib;
		}

		// try get interface
		newmod.load = (void (*)()) dlsym(newmod.dlp, "load");
		if (!newmod.load) goto bad_mod;
		newmod.unload = (void (*)()) dlsym(newmod.dlp, "unload");
		if (!newmod.unload) goto bad_mod;
		newmod.serve = (const char * (*)(const char *)) dlsym(newmod.dlp, "serve");
		if (!newmod.serve) goto bad_mod;

		// load module
		printf("loading module...");
		newmod.load();
		puts("done.");

		// register module
		printf("registering module...");
		modules[modname] = newmod;
		puts("done.");

		return;
bad_mod:
		puts("BAD module.");
		errormsg = dlerror();
		if (errormsg)
			puts(errormsg);
close_lib:
		if (dlclose(newmod.dlp)) {
			puts("fail closing shared library.");
			errormsg = dlerror();
			if (errormsg)
				puts(errormsg);
		}
	} else {
		puts("fail.");
		errormsg = dlerror();
		if (errormsg)
			puts(errormsg);
	}
}
