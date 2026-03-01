#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

#define OUT "output"

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

int main(const int argc, const char** argv)
{
	makeargs_add_target(save);
	makeargs_add_target(snap);
	makeargs_nocolor = 1;
	makeargs_set("self", argv[0]);

	MAKEARGS(argc, argv);
	return 0;
}
