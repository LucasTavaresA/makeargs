#include "../../makeargs.c"

#define OUT "output"
#define MAKEARGS_TARGETS \
	MAKEARGS_TARGET(save)  \
	MAKEARGS_TARGET(snap)

void snap()
{
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "%s foo > " OUT " 2>&1", makeargs_get("self"));
	system(cmd);

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
