#include <stdio.h>

FILE* out;
#define LOG_STDOUT out
#include "../../log.c"

#define MAKEARGS_TARGET_CALL(target) LOG_MSG(#target "()\n");
#define MAKEARGS_IMPLEMENTATION
#define MAKEARGS_TARGETS                                              \
	MAKEARGS_TARGET(empty)                                              \
	MAKEARGS_TARGET(desc, "has a description")                          \
	MAKEARGS_TARGET(nodesc, "")                                         \
	MAKEARGS_TARGET(build, "builds main", "main", "main.c", "common.c") \
	MAKEARGS_TARGET(run, "calls build", "", "main")
#include "../../makeargs.c"

#define SEPARATOR \
	"--------------------------------------------------------------------------------\n"

#define STR_ARRAY(...)                    \
	(str_array_t)                           \
	{                                       \
		MAKEARGS_PARAM_STR_ARRAY(__VA_ARGS__) \
	}

typedef struct
{
	const size_t size;
	const char* const* str;
} str_array_t;

const str_array_t test_args[] = {
		STR_ARRAY("./test"),
		STR_ARRAY("./test", "build"),
		STR_ARRAY("./test", "build", "run"),
		STR_ARRAY("./test", "--"),
		STR_ARRAY("./test", "build", "--"),
		STR_ARRAY("./test", "run", "build", "--"),
		STR_ARRAY("./test", "--", "-s"),
		STR_ARRAY("./test", "build", "--", "-s"),
		STR_ARRAY("./test", "build", "run", "--", "-s"),
		STR_ARRAY("./test", "--", "-s", "--test"),
		STR_ARRAY("./test", "build", "--", "-s"),
		STR_ARRAY("./test", "run", "build", "--", "-s", "--test"),
		STR_ARRAY("./test", "CONTAINER=docker"),
		STR_ARRAY("./test", "run", "CONTAINER=docker", "build"),
		STR_ARRAY("./test", "CONTAINER="),
		STR_ARRAY("./test", "run", "CONTAINER=", "build"),
		STR_ARRAY("./test", "build", "VAR="),
		STR_ARRAY("./test", "VAR=", "build"),
		STR_ARRAY("./test", "build", "--", "DEBUG=1"),
		STR_ARRAY("./test", "build", "--", "DEBUG="),
		STR_ARRAY("./test", "FOO=bar", "--", "-s"),
		STR_ARRAY("./test", "--", "FOO=bar"),
		STR_ARRAY("./test", "FOO=", "--", "-s"),
		STR_ARRAY("./test", "--", "FOO="),
};
const size_t test_args_count = MAKEARGS_COUNTOF(test_args);

void print_array(const size_t size, const char* const array[])
{
	int i = 0;

	while (i < size)
	{
		LOG_MSG("%s ", array[i]);
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

		for (int i = 0; i < test_args_count; i++)
		{
			print_array(test_args[i].size, test_args[i].str);
			int ret = makeargs_run_targets(test_args[i].size, test_args[i].str);
			LOG_MSG("%d <- makeargs_run_targets()\n", ret);

			ret = makeargs_set_vars(test_args[i].size, test_args[i].str);
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
