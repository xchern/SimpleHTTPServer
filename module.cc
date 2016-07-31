#include <stdio.h>
#include <dlfcn.h>
#include <string>
#include <map>

struct ModItem {
	void * dlp;
	void (*load)(void);
	void (*unload)(void);
	const char * (*serve)(const char *);
};

static std::map <std::string, struct ModItem> modules;

inline bool modExistP(const char * name) {
	std::string modname(name);
	return (modules.count(modname) > 0);
}

const char * modServe(const char * name, const char * param) {
	std::string modname(name);
	return (modules[modname].serve(param));
}

void doUnload(const char * name) {
	puts("[unload]");
	printf("try to unload module \'%s\'\n", name);
	std::string modname(name);
	// check if module exists
	if (!modExistP(name)) {
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

void doLoad(const char * path) {
	puts("[load]");
	struct ModItem newmod;
	std::string modname;
	char * errormsg;
	printf("try to load from file %s...", path);
	// try load shared lib
	newmod.dlp = dlopen(path, RTLD_LAZY);
	if (newmod.dlp) {
		puts("done.");
		// try get module information
		puts("try to getmodule information.");
		const char * (* getName)(void) = (const char * (*)()) dlsym(newmod.dlp, "getName");
		const char * name;
		if (!getName) goto bad_mod;
		name = getName();
		printf("module name: %s\n", name);

		if (modExistP(name)) { // abort if name exists
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
		modname = name;
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
			puts("fail unload shared library.");
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
