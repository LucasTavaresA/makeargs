// tests if -o <file> is making exceptions properly, even with `-B` to always build
#include "../../makeargs.c"

#define MAKEARGS_TARGETS                                                  \
	MAKEARGS_TARGET(all, "", "all.txt", "out1.txt", "out2.txt", "out3.txt") \
	MAKEARGS_TARGET(out1, "", "out1.txt", "in1.txt")                        \
	MAKEARGS_TARGET(out2, "", "out2.txt", "in2.txt")                        \
	MAKEARGS_TARGET(out3, "", "out3.txt", "in3.txt")                        \
	MAKEARGS_TARGET(snap, "outputs results from makeargs and make")         \
	MAKEARGS_TARGET(save, "saves output to expected")

void all()
{
	system("echo \"Building all\"");
	system("cat out1.txt out2.txt out3.txt > all.txt");
}

void out1()
{
	system("echo \"Building out1.txt from in1.txt\"");
	system("cp in1.txt out1.txt");
}

void out2()
{
	system("echo \"Building out2.txt from in2.txt\"");
	system("cp in2.txt out2.txt");
}

void out3()
{
	system("echo \"Building out3.txt from in3.txt\"");
	system("cp in3.txt out3.txt");
}

#define OUT "output"

void snap()
{
	{
		system("rm -f out1.txt");

		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s -B -o out1.txt all > " OUT " 2>&1",
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
