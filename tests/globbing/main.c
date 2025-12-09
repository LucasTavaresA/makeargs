#include <stdio.h>

extern void a();
extern void b();

int main(int argc, char* argv[])
{
	a();
	b();
	printf("main\n");
	return 0;
}
