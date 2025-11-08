#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../span/span.c"

FILE* out;
#define LOG_STDOUT out
#include "../log.c"

#define MAKEARGS_TARGET_CALL(target) LOG_MSG(#target "()\n");
#define MAKEARGS_IMPLEMENTATION
#define MAKEARGS_TARGETS                                              \
	MAKEARGS_TARGET(empty)                                              \
	MAKEARGS_TARGET(desc, "has a description")                          \
	MAKEARGS_TARGET(nodesc, "")                                         \
	MAKEARGS_TARGET(build, "builds main", "main", "main.c", "common.c") \
	MAKEARGS_TARGET(run, "calls build", "", "main")
#include "../makeargs.c"

#define SEPARATOR \
	"--------------------------------------------------------------------------------\n"

typedef SPAN_T(string_span) ss_span;
#define SS_SPAN(...) (ss_span) SPAN(string_span, __VA_ARGS__)

const ss_span test_args =
		SS_SPAN(STRING_SPAN("./test"),
						STRING_SPAN("./test", "build"),
						STRING_SPAN("./test", "build", "run"),
						STRING_SPAN("./test", "--"),
						STRING_SPAN("./test", "build", "--"),
						STRING_SPAN("./test", "run", "build", "--"),
						STRING_SPAN("./test", "--", "-s"),
						STRING_SPAN("./test", "build", "--", "-s"),
						STRING_SPAN("./test", "build", "run", "--", "-s"),
						STRING_SPAN("./test", "--", "-s", "--test"),
						STRING_SPAN("./test", "build", "--", "-s"),
						STRING_SPAN("./test", "run", "build", "--", "-s", "--test"),
						STRING_SPAN("./test", "CONTAINER=docker"),
						STRING_SPAN("./test", "run", "CONTAINER=docker", "build"),
						STRING_SPAN("./test", "CONTAINER="),
						STRING_SPAN("./test", "run", "CONTAINER=", "build"),
						STRING_SPAN("./test", "build", "VAR="),
						STRING_SPAN("./test", "VAR=", "build"),
						STRING_SPAN("./test", "build", "--", "DEBUG=1"),
						STRING_SPAN("./test", "build", "--", "DEBUG="),
						STRING_SPAN("./test", "FOO=bar", "--", "-s"),
						STRING_SPAN("./test", "--", "FOO=bar"),
						STRING_SPAN("./test", "FOO=", "--", "-s"),
						STRING_SPAN("./test", "--", "FOO="), );

void print_args(string_span ss)
{
	int i = 0;

	while (i < ss.size)
	{
		LOG_MSG("%s ", ss.data[i]);
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

		for (int i = 0; i < test_args.size; i++)
		{
			print_args(test_args.data[i]);
			int ret =
					makeargs_run_targets(test_args.data[i].size, test_args.data[i].data);
			LOG_MSG("%d <- makeargs_run_targets()\n", ret);

			ret = makeargs_set_vars(test_args.data[i].size, test_args.data[i].data);
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
