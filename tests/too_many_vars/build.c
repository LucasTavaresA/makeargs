#define MAKEARGS_MAX_VARS 3
#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

#define OUT "output"

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

int main(const int argc, const char** argv)
{
	makeargs_add_target(snap);
	makeargs_add_target(save);
	makeargs_nocolor = 1;
	makeargs_set("self", argv[0]);

	makeargs_set_vars(argc, argv);
	makeargs_run_targets(argc, argv);
	return 0;
}
