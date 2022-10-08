#include <readlines/readlines.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>

bool readlines_is_closed(const char *str)
{
    int depth = 0;

    while (*str)
    {
        if (*str == '(')
            depth++;
        else if (*str == ')' && depth > 0)
            depth--;
        str++;
    }

    return depth <= 0;
}

int readlines_bind_cr(int count, int key)
{
    rl_insert_text("\n");

    if (readlines_is_closed(rl_line_buffer))
    {
        rl_done = 1;
        printf("\n");
    }
    else
    {
        // printf(">");
    }
}

int readlines_startup_hook(void)
{
    rl_bind_key('\n', readlines_bind_cr);
    rl_bind_key('\r', readlines_bind_cr);
}

void readlines_init()
{
    rl_readline_name = "readlines";
    rl_startup_hook = readlines_startup_hook;
}

char *readlines(const char *prompt)
{
    char *line;

    if ((line = readline(prompt)) == NULL)
    {
        return NULL;
    }

    printf("\"%s\"\n", line);
}