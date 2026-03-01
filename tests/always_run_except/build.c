// tests if -o <file> is making exceptions properly, even with `-B` to always build
#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

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

	{
		system("rm -f out1.txt");

		char cmd[1024];
		snprintf(cmd, sizeof(cmd),
						 "%s -B -o out1.txt -W out1.txt all >> " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		system("rm -f out1.txt");

		char cmd[1024];
		snprintf(cmd, sizeof(cmd),
						 "%s -B -W out1.txt -o out1.txt all >> " OUT " 2>&1",
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
	makeargs_add_target(all, .output = "all.txt", MAKEARGS_DEPS("out*.txt"));
	makeargs_add_target(out1, .output = "out1.txt", MAKEARGS_DEPS("in1.txt"));
	makeargs_add_target(out2, .output = "out2.txt", MAKEARGS_DEPS("in2.txt"));
	makeargs_add_target(out3, .output = "out3.txt", MAKEARGS_DEPS("in3.txt"));
	makeargs_add_target(snap,
											.description = "outputs results from makeargs and make");
	makeargs_add_target(save, .description = "saves output to expected");
	makeargs_nocolor = 1;
	makeargs_set("self", argv[0]);

	MAKEARGS(argc, argv);
	return 0;
}
