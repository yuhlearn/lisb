#include <common/common.h>
#include <chunk/chunk.h>
#include <debug/debug.h>
#include <vm/vm.h>

#include <readline/readline.h>
#include <readline/history.h>
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

int rlgets(char *buffer, const int n, const int line_count)
{
	char prompt[256];
	char *line = NULL;
	int length = 0;

	sprintf(prompt, "> ");
	line = readline(prompt);

	if (line && *line)
		add_history(line);

	if (line)
	{
		length = strlen(line);

		if (length >= n)
		{
			free(line);
			return 0;
		}

		strncpy(buffer, line, length);
		buffer[length++] = '\n';
		free(line);
	}

	return length;
}

static void main_repl()
{
	char buffer[1024];
	size_t start = 0, length = 0;
	int line_count;

	for (;;)
	{
		memset(buffer, '\0', sizeof(buffer));
		line_count = 1;
		do
		{
			start += length;
			if (!(length = rlgets(buffer + start,
								  sizeof(buffer) - start - 1,
								  line_count)))
			{
				printf("Input buffer overflow. Terminating.\n");
				return;
			}
			line_count++;
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

	if (result == VM_COMPILE_ERROR)
		exit(65);
	if (result == VM_RUNTIME_ERROR)
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
