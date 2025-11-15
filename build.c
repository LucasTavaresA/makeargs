// includes definitions, but no implementations yet
#include "makeargs.c"

// targets are defined at compile time
// name is required, the rest is optional, you can have 10 dependencies
// when calling a target needs a dependency, makeargs calls what produces that output
// MAKEARGS_TARGET(name, description, output, dependencies...)
#define MAKEARGS_TARGETS                                          \
	MAKEARGS_TARGET(build, "", "main", "main.c", "utils.c")         \
	MAKEARGS_TARGET(run, "", "", "main")                            \
	MAKEARGS_TARGET(format, "format all c files with clang-format") \
	MAKEARGS_TARGET(clean, "remove all the build files")

// these explain themselves
void format()
{
	system("clang-format -i ./*.c ./tests/**/*.c");
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
	char* cc = makeargs_get("CC");

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "%s %s %s -o %s main.c", cc, cflags, warnings,
					 out);
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
	// for overridable values, just use makeargs_set() before everything:
	makeargs_set("OUT", "main");
	makeargs_set("WARNINGS", "-Wall -Wextra -pedantic");
	makeargs_set("CFLAGS", "-std=c99 -O3 -static");
	makeargs_set("CC", "cc");

	// macro that replicates make behaviour by:
	// setting all the variables and flags then runs the targets
	MAKEARGS(argc, argv);
	return 0;
}
