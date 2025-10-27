// makeargs.c does not include anything by itself, so it needs these includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// log.c is a bunch of simple logging macros, it uses stdio.h
#include "log.c"
// includes usefull definitions, but no implementations yet
#include "makeargs.c"

// to avoid executing arbitrary c functions, your targets are defined at compile time:
// MAKEARGS_TARGET(name, description)
#define MAKEARGS_TARGETS                                          \
	MAKEARGS_TARGET(build)                                          \
	MAKEARGS_TARGET(run)                                            \
	MAKEARGS_TARGET(format, "format all c files with clang-format") \
	MAKEARGS_TARGET(clean, "remove all the build files")

// these explain themselves
void format()
{
	system("clang-format -i *.c");
}

void clean()
{
	char* out = makeargs_get("OUT");
	char cmd[1024];

	snprintf(cmd, sizeof(cmd), "rm -f %s", out);
	system(cmd);
}

void build()
{
	clean();

	char* cflags = makeargs_get("CFLAGS");
	char* warnings = makeargs_get("WARNINGS");
	char* out = makeargs_get("OUT");

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "gcc %s %s -o %s main.c", cflags, warnings, out);
	system(cmd);
}

void run()
{
	char* out = makeargs_get("OUT");

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "./%s", out);
	system(cmd);
}

// this will now include the implementations, using the definitions above
#define MAKEARGS_IMPLEMENTATION
#include "makeargs.c"

int main(const int argc, const char** argv)
{
	/// the order is important since all variables are set using makeargs_set()
	/// the order used below matches makefile behaviour

	// for overridable values, just use makeargs_set() before everything:
	makeargs_set("OUT", "program");
	makeargs_set("WARNINGS", "-Wall -Wextra -pedantic");
	makeargs_set("CFLAGS", "-std=c99 -O3 -static");

	// gets all the environment variables, and sets them with makeargs_set()
	makeargs_getenv();
	// sets the variables passed from the command line
	makeargs_set_vars(argc, argv);
	// runs the targets passed from the command line
	makeargs_run_targets(argc, argv);
	return 0;
}
