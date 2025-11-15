#include "../../makeargs.c"

#define MAKEARGS_TARGETS                     \
	MAKEARGS_TARGET(builda, "", "a", "b")      \
	MAKEARGS_TARGET(buildb, "", "b", "c")      \
	MAKEARGS_TARGET(buildc, "", "c", "a")      \
	MAKEARGS_TARGET(snap, "saves test output") \
	MAKEARGS_TARGET(save, "saves output to expected")

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
