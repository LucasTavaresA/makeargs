#include <stdio.h>

FILE* out;
#define MAKEARGS_STDOUT out

#define MAKEARGS_TARGET_CALL(target) MAKEARGS_MSG("%s()\n", (target)->name);
#define MAKEARGS_IMPLEMENTATION
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

void empty() {}
void desc() {}
void nodesc() {}
void build() {}
void run() {}

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
		MAKEARGS_MSG("%s ", array[i]);
		i++;
	}
	MAKEARGS_MSG("\n");
}

int main(int argc, const char** argv)
{
	makeargs_add_target(empty);
	makeargs_add_target(desc, .description = "has a description");
	makeargs_add_target(nodesc);
	makeargs_add_target(build, .description = "builds main", .output = "main",
											MAKEARGS_DEPS("main.c", "common.c"));
	makeargs_add_target(run, .description = "calls build", MAKEARGS_DEPS("main"));
	makeargs_nocolor = 1;

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
			MAKEARGS_MSG("%d <- makeargs_run_targets()\n", ret);

			ret = makeargs_set_vars(test_args[i].size, test_args[i].str);
			MAKEARGS_MSG("%d <- makeargs_set_vars()\n", ret);

			for (int i = 0; i < makeargs_vars_count; i++)
			{
				MAKEARGS_MSG("%s=\"%s\"\n", makeargs_vars[i].name,
										 makeargs_vars[i].value);
			}

			makeargs_vars_count = 0;
			MAKEARGS_MSG(SEPARATOR);
		}
	}

	fclose(out);

	system("diff output expected");
	return 0;
}
