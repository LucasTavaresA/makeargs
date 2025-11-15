#include "../../makeargs.c"

#define MAKEARGS_MAX_VARS 3
#define OUT "output"
#define MAKEARGS_TARGETS \
	MAKEARGS_TARGET(snap)  \
	MAKEARGS_TARGET(save)

void snap()
{
	char cmd[1024];
	snprintf(cmd, sizeof(cmd),
					 "%s ONE=one TWO=two THREE=three FOUR=four > " OUT " 2>&1",
					 makeargs_get("self"));
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
