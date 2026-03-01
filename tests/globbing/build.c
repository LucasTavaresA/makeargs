#include <unistd.h>

#define MAKEARGS_IMPLEMENTATION
#include "../../makeargs.c"

// NOTE(LucasTA): separated cause tree-sitter thinks globs are a comment
#define O_FILES \
	"**/"         \
	"*.o"

void builda()
{
	system("gcc -c folder/a.c -o folder/a.o");
}

void buildb()
{
	system("gcc -c folder/b.c -o folder/b.o");
}

void build()
{
	system("gcc -c main.c -o main.o");
	system("gcc folder/a.o folder/b.o main.o -o main");
}

#define OUT "output"

void snap()
{
	system("rm -f " O_FILES " main " OUT);
	system("touch " OUT);

	{
		system("printf 'builds everything:\n' >>" OUT);
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s build >> " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	{
		system("printf 'everything is built (does nothing):\n' >>" OUT);
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s build >> " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	sleep(1);
	system("touch folder/a.c");
	{
		system("printf 'a.c was touched 😳:\n' >>" OUT);
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s build >> " OUT " 2>&1",
						 makeargs_get("self"));
		system(cmd);
	}

	sleep(1);
	system("touch folder/b.c");
	{
		system("printf 'b.c was touched 😳:\n' >>" OUT);
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "%s build >> " OUT " 2>&1",
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
	makeargs_add_target(builda, .output = "folder/a.o",
											MAKEARGS_DEPS("folder/a.c"));
	makeargs_add_target(buildb, .output = "folder/b.o",
											MAKEARGS_DEPS("folder/b.c"));
	makeargs_add_target(build, .output = "main", MAKEARGS_DEPS(O_FILES));
	makeargs_add_target(snap);
	makeargs_add_target(save);
	makeargs_nocolor = 1;
	makeargs_set("self", argv[0]);

	MAKEARGS(argc, argv);
	return 0;
}
