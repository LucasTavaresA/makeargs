#include "../../makeargs.c"

#define OUT "output"
#define MAKEARGS_TARGETS   \
	MAKEARGS_TARGET(setname) \
	MAKEARGS_TARGET(argname) \
	MAKEARGS_TARGET(flag)    \
	MAKEARGS_TARGET(save)    \
	MAKEARGS_TARGET(snap)

void setname()
{
	makeargs_set("", "nothing?");
}

void argname()
{
	int argc = 2;
	const char* argv[] = {makeargs_get("self"), "=BAR"};
	makeargs_set_vars(argc, argv);
}

void flag()
{
	int argc = 2;
	const char* argv[] = {makeargs_get("self"), "-o"};
	makeargs_set_flags(argc, argv);
}

void snap()
{
	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s setname > " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s argname >> " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s flag >> " OUT " 2>&1", makeargs_get("self"));
		system(cmd);
	}

	system("diff " OUT " expected");
}

void save()
{
	snap();
	system("cp " OUT " expected");
}

#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

int main(const int argc, const char** argv)
{
	makeargs_set("self", argv[0]);

	if (argc == 1)
	{
		snap();
		exit(0);
	}

	MAKEARGS(argc, argv);
	return 0;
}
