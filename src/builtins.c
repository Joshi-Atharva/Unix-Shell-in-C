#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "builtins.h"

int is_builtin(const char *cmd) {
    if (!cmd) return 0;
    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "pwd") == 0);
}

int execute_builtin(int argc, char **argv) {
    if (!argv || !argv[0]) return 0;

    if (strcmp(argv[0], "exit") == 0) {
        printf("Exiting shell...\n");
        int exit_code = 0;
        if (argc > 1 && argv[1]) {
            exit_code = atoi(argv[1]);
        }
        exit(exit_code);
    }

    if (strcmp(argv[0], "cd") == 0) {
        const char *target = NULL;
        if (argc < 2 || argv[1] == NULL || strcmp(argv[1], "~") == 0) {
            target = getenv("HOME");
            if (!target) {
                target = ".";
            }
        } else {
            target = argv[1];
        }

        if (chdir(target) != 0) {
            fprintf(stderr, "cd: %s: No such file or directory\n", target);
            return 1;
        }
        return 0;
    }

    if (strcmp(argv[0], "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
            return 0;
        } else {
            perror("pwd");
            return 1;
        }
    }

    return 0;
}
