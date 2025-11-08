#include <stdio.h>

int printn(int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		printf("%d, ", i);
	}
	printf("\n");
	return 0;
}
