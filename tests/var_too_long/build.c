#define MAKEARGS_VAR_LENGTH 25
#define OUT "output"

#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

void append()
{
	char a[MAKEARGS_VAR_LENGTH + 2] = {[0 ... MAKEARGS_VAR_LENGTH] = 'a', 0};
	makeargs_set("name", "");
	makeargs_append("name", a);
}

void setval()
{
	char a[MAKEARGS_VAR_LENGTH + 2] = {[0 ... MAKEARGS_VAR_LENGTH] = 'a', 0};
	makeargs_set("name", a);
}

void setname()
{
	char a[MAKEARGS_VAR_LENGTH + 2] = {[0 ... MAKEARGS_VAR_LENGTH] = 'a', 0};
	makeargs_set(a, "value");
}

void snap()
{
	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s append > " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s setname >> " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s setval >> " OUT " 2>&1",
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
	makeargs_add_target(append);
	makeargs_add_target(setval);
	makeargs_add_target(setname);
	makeargs_add_target(snap);
	makeargs_add_target(save);
	makeargs_nocolor = 1;
	makeargs_set("self", argv[0]);

	makeargs_run_targets(argc, argv);
	return 0;
}
