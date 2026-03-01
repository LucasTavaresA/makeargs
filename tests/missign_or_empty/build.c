#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

#define OUT "output"

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

int main(const int argc, const char** argv)
{
	makeargs_add_target(setname);
	makeargs_add_target(argname);
	makeargs_add_target(flag);
	makeargs_add_target(save);
	makeargs_add_target(snap);
	makeargs_nocolor = 1;
	makeargs_set("self", argv[0]);

	MAKEARGS(argc, argv);
	return 0;
}
