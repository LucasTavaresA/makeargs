#include "../../makeargs.c"

#define MAKEARGS_VAR_LENGTH 25
#define OUT "output"
#define MAKEARGS_TARGETS   \
	MAKEARGS_TARGET(append)  \
	MAKEARGS_TARGET(setval)  \
	MAKEARGS_TARGET(setname) \
	MAKEARGS_TARGET(snap)    \
	MAKEARGS_TARGET(save)

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
