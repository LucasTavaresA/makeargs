#define MAKEARGS_IMPLEMENTATION
#include "makeargs.c"

void format(void)
{
	system("clang-format -i ./*.c ./tests/**/*.c");
}

void clean(void)
{
	char* out = makeargs_get("OUT");
	char cmd[1024];

	snprintf(cmd, sizeof(cmd), "rm -f %s", out);
	system(cmd);
}

void build(void)
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

void run(void)
{
	char* out = makeargs_get("OUT");

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "./%s", out);
	system(cmd);
}

int main(const int argc, const char** argv)
{
	// for overridable values, just use makeargs_set() before everything:
	makeargs_set("OUT", "main");
	makeargs_set("WARNINGS", "-Wall -Wextra -pedantic");
	makeargs_set("CFLAGS", "-std=c99 -O3 -static");
	makeargs_set("CC", "cc");

	// the target function is required, the rest is optional
	makeargs_add_target(build, .output = "main",
											MAKEARGS_DEPS("main.c", "utils.c"));
	makeargs_add_target(run, MAKEARGS_DEPS("main"));
	makeargs_add_target(format,
											.description = "format all c files with clang-format");
	makeargs_add_target(clean, .description = "remove all the build files");

	// macro that replicates make behaviour by:
	// setting all the variables and flags then runs the targets
	MAKEARGS(argc, argv);
	return 0;
}
