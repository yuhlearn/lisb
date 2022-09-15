#include <module/module.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Requires two integer arguments\n");
		return 0;
	}

	int a = atoi(argv[1]);
	int b = atoi(argv[2]);
	
	printf("%d + %d = %d\n", a, b, module_add(a ,b));
    return 0;
}
