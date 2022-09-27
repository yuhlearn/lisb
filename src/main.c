#include <common/common.h>
#include <chunk/chunk.h>
#include <debug/debug.h>
#include <vm/vm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_closed(char *str)
{
	int depth = 0;

	while (*str)
	{
		if (*str == '(')
			depth++;
		else if (*str == ')')
			depth--;
		str++;
	}

	return depth <= 0;
}

static void main_repl()
{
	char buffer[1024];
	size_t start = 0, length = 0;
	int line;

	for (;;)
	{
		line = 1;
		do
		{
			start += length;
			printf("%d> ", line);
			if (!fgets(buffer + start, sizeof(buffer), stdin))
			{
				printf("\n");
				return;
			}
			length = strlen(buffer + start);
			line++;
		} while (!is_closed(buffer));

		start = length = 0;
		vm_interpret(buffer);
	}
}

static char *main_read_file(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file == NULL)
	{
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t file_size = ftell(file);
	rewind(file);

	char *buffer = (char *)malloc(file_size + 1);

	if (buffer == NULL)
	{
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	}

	size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

	if (bytes_read < file_size)
	{
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}

	buffer[bytes_read] = '\0';

	fclose(file);
	return buffer;
}

static void main_run_file(const char *path)
{
	char *source = main_read_file(path);
	InterpretResult result = vm_interpret(source);
	free(source);

	if (result == INTERPRET_COMPILE_ERROR)
		exit(65);
	if (result == INTERPRET_RUNTIME_ERROR)
		exit(70);
}

int main(int argc, char **argv)
{
	vm_init_vm();

	if (argc == 1)
	{
		main_repl();
	}
	else if (argc == 2)
	{
		main_run_file(argv[1]);
	}
	else
	{
		fprintf(stderr, "Usage: lisb [path]\n");
		exit(64);
	}

	vm_free_vm();
	return 0;
}
