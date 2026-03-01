#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

void builda() {}
void buildb() {}
void buildc() {}

#define OUT "output"

void snap()
{
	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s builda > " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s buildb >> " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s buildc >> " OUT " 2>&1",
						 makeargs_get("self"));
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
	makeargs_add_target(builda, .output = "a", MAKEARGS_DEPS("b"));
	makeargs_add_target(buildb, .output = "b", MAKEARGS_DEPS("c"));
	makeargs_add_target(buildc, .output = "c", MAKEARGS_DEPS("a"));
	makeargs_add_target(snap, .description = "saves test output");
	makeargs_add_target(save, .description = "saves output to expected");
	makeargs_nocolor = 1;
	makeargs_set("self", argv[0]);

	MAKEARGS(argc, argv);
	return 0;
}
