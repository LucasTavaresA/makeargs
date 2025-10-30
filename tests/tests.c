#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* out;
#define LOG_STDOUT out
#include "../log.c"

#define MAKEARGS_TARGET_CALL(target) LOG_MSG(#target "()\n");
#define MAKEARGS_TARGETS \
	MAKEARGS_TARGET(build) \
	MAKEARGS_TARGET(run)

#define MAKEARGS_IMPLEMENTATION
#include "../makeargs.c"

#define SEPARATOR \
	"--------------------------------------------------------------------------------\n"
#define ARGS(...)                                               \
	{                                                             \
		sizeof((const char*[]){__VA_ARGS__}) / sizeof(const char*), \
				(const char*[])                                         \
		{                                                           \
			__VA_ARGS__                                               \
		}                                                           \
	}

typedef struct
{
	int argc;
	const char** argv;
} args;

const args test_args[] = {
		ARGS("./test"),
		ARGS("./test", "build"),
		ARGS("./test", "build", "run"),
		ARGS("./test", "--"),
		ARGS("./test", "build", "--"),
		ARGS("./test", "run", "build", "--"),
		ARGS("./test", "--", "-s"),
		ARGS("./test", "build", "--", "-s"),
		ARGS("./test", "build", "run", "--", "-s"),
		ARGS("./test", "--", "-s", "--test"),
		ARGS("./test", "build", "--", "-s"),
		ARGS("./test", "run", "build", "--", "-s", "--test"),
		ARGS("./test", "CONTAINER=docker"),
		ARGS("./test", "run", "CONTAINER=docker", "build"),
		ARGS("./test", "CONTAINER="),
		ARGS("./test", "run", "CONTAINER=", "build"),
		ARGS("./test", "build", "VAR="),
		ARGS("./test", "VAR=", "build"),
		ARGS("./test", "build", "--", "DEBUG=1"),
		ARGS("./test", "build", "--", "DEBUG="),
		ARGS("./test", "FOO=bar", "--", "-s"),
		ARGS("./test", "--", "FOO=bar"),
		ARGS("./test", "FOO=", "--", "-s"),
		ARGS("./test", "--", "FOO="),
};

void print_args(const int argc, const char** argv)
{
	int i = 0;

	while (i < argc)
	{
		LOG_MSG("%s ", argv[i]);
		i++;
	}
	LOG_MSG("\n");
}

int main(int argc, const char** argv)
{
	if (argc > 1 && strcmp(argv[1], "save") == 0)
	{
		out = fopen("expected", "w");
	}
	else
	{
		out = fopen("output", "w");
	}

	{
		makeargs_vars_count = 0;

		for (int i = 0; i < sizeof(test_args) / sizeof(test_args[0]); i++)
		{
			print_args(test_args[i].argc, test_args[i].argv);
			int ret = makeargs_run_targets(test_args[i].argc, test_args[i].argv);
			LOG_MSG("%d <- makeargs_run_targets()\n", ret);

			ret = makeargs_set_vars(test_args[i].argc, test_args[i].argv);
			LOG_MSG("%d <- makeargs_set_vars()\n", ret);

			for (int i = 0; i < makeargs_vars_count; i++)
			{
				LOG_MSG("%s=\"%s\"\n", makeargs_vars[i].name, makeargs_vars[i].value);
			}

			makeargs_vars_count = 0;
			LOG_MSG(SEPARATOR);
		}
	}

	fclose(out);

	system("diff output expected");
	return 0;
}
