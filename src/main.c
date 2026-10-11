#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>

extern FILE *yyin;
extern int yyparse(void);

static void print_prompt(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s$ ", cwd);
    } else {
        printf("myshell$ ");
    }
    fflush(stdout);
}

int main(void) {
    // Shell process ignores SIGINT (Ctrl+C) and SIGTSTP (Ctrl+Z)
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    char *line = NULL;
    size_t len = 0;

    while (1) {
        // Non-blocking reap of any background child processes
        while (waitpid(-1, NULL, WNOHANG) > 0);

        print_prompt();

        ssize_t nread = getline(&line, &len, stdin);
        if (nread == -1) {
            // EOF reached (Ctrl+D)
            printf("\nExiting shell...\n");
            break;
        }

        // Check if the line consists entirely of whitespace
        int is_empty = 1;
        for (ssize_t i = 0; i < nread; i++) {
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r') {
                is_empty = 0;
                break;
            }
        }
        if (is_empty) {
            continue;
        }

        // Store command line into a temporary file for Flex/Bison parsing
        FILE *temp_file = tmpfile();
        if (!temp_file) {
            perror("tmpfile");
            continue;
        }

        fwrite(line, 1, (size_t)nread, temp_file);
        // Ensure line ends with a newline for grammar production rules
        if (line[nread - 1] != '\n') {
            fputc('\n', temp_file);
        }
        rewind(temp_file);

        // Feed temporary file stream to the parser
        yyin = temp_file;
        yyparse();

        fclose(temp_file);
    }

    free(line);
    return EXIT_SUCCESS;
}
