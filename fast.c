#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define FAST_READ_BUFSIZE 1024
#define FAST_TOK_BUFSIZE 64
#define FAST_TOK_DELIM " \t\r\n\a"

char *current_user;

/*
  Function Declarations for builtin fireshell commands:
 */
int fire_cd(char **args);
int fire_help(char **args);
int fire_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
};

int (*builtin_func[])(char **) = {
    &fire_cd,
    &fire_help,
    &fire_exit,
};

// returns the number of builtin functions
int fire_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/*
 * change dir implementation
 * using chdir syscall 
 */
int fire_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "fireshell: expected some arguments for cd command\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            // execute chdir command
            perror("fireshell");
        }
    }
    return 1;
}

int fire_help(char **args)
{
    int i;
    int built_in = fire_num_builtins();

    printf("Welcome to fireshell builtin help\n");
    printf("This is a list of currently implemented built in command:\n");

    for (i = 0; i < built_in; i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for more information on other programs.\n");
    return 1;
}

/*
 * fire exit
 */
int fire_exit(char **args)
{
    return 0;
}

/*
 * reading a line from fast shell ui
 * @args void (None)
 * @return char* represents user input
 */
char *fast_read_line(void)
{
    int bufsize = FAST_READ_BUFSIZE;
    int i = 0; // reading loop index
    char *buffer = (char *)malloc(sizeof(char) * bufsize);
    int c; // charcter to read at a time

    while (1)
    {
        c = getchar();

        // case input charcter was ENTER (EOF)
        // finishing reading routine with '\0' charcter
        if (c == EOF || c == '\n')
        {
            buffer[i] = '\0';
            return buffer;
        }
        else
        {
            buffer[i] = c;
        }
        i++;

        // case user used all the buffer size, realloc
        if (i >= bufsize)
        {
            bufsize += FAST_READ_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "fastshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*
 * parsing a given line into command and it's arguments
 * @args char* line - a line to parse
 * @return char** - parsed line
 */
char **fast_parse_line(char *line)
{
    int bufsize = FAST_TOK_BUFSIZE;
    int i = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "fastshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, FAST_TOK_DELIM);
    while (token != NULL)
    {
        tokens[i] = token;
        i++;

        // check if we exceed the arguments bufsize
        if (i >= bufsize)
        {
            bufsize += FAST_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "fastshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, FAST_TOK_DELIM);
    }
    tokens[i] = NULL;
    return tokens;
}

/*
 * launch a program
 * using fork and then execpv syscalls in order to execute a new 
 * program from fastshell
 *
 * @args char **args - program arguments
 * @return int- launch status
 */
int fast_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();

    if (pid == 0)
    {
        // chile process
        // case could not use exec() syscall
        if (execvp(args[0], args) == -1)
        {
            perror("fireshell");
        }
        // This code is valid since we change image and
        // on success we dont execute this only in case the image didn't changed 
        // after using exec syscall, this code will be executed and terminate child process
        exit(EXIT_FAILURE); 
    }
    else if (pid < 0)
    {
        // fork failure
        perror("fireshell");
    }
    else
    {
        // parent process here
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int fast_execute(char **args)
{
    int i;
    int built_in = fire_num_builtins();
    if (args[0] == NULL)
    {
        // an empty command was entered, handle this somehow ?
        return 1;
    }

    for (i = 0; i < built_in; i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            // found a built in function and execute it
            return (*builtin_func[i])(args);
        }
    }
    // launch a program 
    return fast_launch(args);
}

/*
 * fast_loop
 * here we run fastshell routine which is:
 * (1) read user input
 * (2) parse the input line
 * (3) execute the command
 *
 * @args void (NONE)
 */
void fast_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        // get the current user from env variable - should be replaced
        // put this in a different func asap!!!
        current_user = getenv("USER");
        printf("fastshell~ %s> ", current_user);
        line = fast_read_line();
        args = fast_parse_line(line);
        status = fast_execute(args);

        // may not be required since we are doing reuse
        // anyway investigate this with valgrind
        free(line);
        free(args);

    } while (status);
}

int main(int argc, char **argv)
{
    // Load config files, if any.

    // Run command loop.
    fast_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}