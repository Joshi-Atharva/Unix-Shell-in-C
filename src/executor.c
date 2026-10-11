#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "executor.h"
#include "builtins.h"

int execute_pipeline(Pipeline *pipeline) {
    if (!pipeline || pipeline->cmd_count == 0) {
        return 0;
    }

    int N = pipeline->cmd_count;

    // Fast path: Single builtin command that modifies parent shell state
    if (N == 1 && pipeline->commands[0]->argc > 0 && is_builtin(pipeline->commands[0]->argv[0])) {
        SimpleCommand *sc = pipeline->commands[0];

        // "exit" and "cd" must always run in the parent process
        if (strcmp(sc->argv[0], "exit") == 0 || strcmp(sc->argv[0], "cd") == 0) {
            return execute_builtin(sc->argc, sc->argv);
        }

        // Other builtins (e.g., "pwd") can support redirection directly in parent
        int saved_stdout = -1;
        if (sc->outfile) {
            int flags = O_WRONLY | O_CREAT | (sc->append_out ? O_APPEND : O_TRUNC);
            int fd = open(sc->outfile, flags, 0666);
            if (fd < 0) {
                perror(sc->outfile);
                return 1;
            }
            saved_stdout = dup(STDOUT_FILENO);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        int status = execute_builtin(sc->argc, sc->argv);

        if (saved_stdout >= 0) {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }
        return status;
    }

    // Allocate pipes for pipeline of N commands: requires N - 1 pipes
    int (*pipes)[2] = NULL;
    if (N > 1) {
        pipes = malloc(sizeof(int[2]) * (N - 1));
        if (!pipes) {
            perror("malloc pipes");
            return 1;
        }

        for (int i = 0; i < N - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                for (int j = 0; j < i; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                free(pipes);
                return 1;
            }
        }
    }

    pid_t *pids = malloc(sizeof(pid_t) * N);
    if (!pids) {
        perror("malloc pids");
        if (pipes) {
            for (int i = 0; i < N - 1; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            free(pipes);
        }
        return 1;
    }

    // Fork and execute each command stage
    for (int i = 0; i < N; i++) {
        SimpleCommand *sc = pipeline->commands[i];

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            break;
        }

        if (pids[i] == 0) {
            // Child process: restore default signal handling
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            // 1. Setup STDIN
            if (sc->infile) {
                int fd_in = open(sc->infile, O_RDONLY);
                if (fd_in < 0) {
                    perror(sc->infile);
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            } else if (i > 0) {
                // Read from previous pipe stage
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // 2. Setup STDOUT
            if (sc->outfile) {
                int flags = O_WRONLY | O_CREAT | (sc->append_out ? O_APPEND : O_TRUNC);
                int fd_out = open(sc->outfile, flags, 0666);
                if (fd_out < 0) {
                    perror(sc->outfile);
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            } else if (i < N - 1) {
                // Write to next pipe stage
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // 3. Setup STDERR
            if (sc->errfile) {
                int fd_err = open(sc->errfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd_err < 0) {
                    perror(sc->errfile);
                    exit(1);
                }
                dup2(fd_err, STDERR_FILENO);
                close(fd_err);
            }

            // 4. Close all pipe descriptors in child
            if (N > 1) {
                for (int j = 0; j < N - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            // 5. Execute command
            if (sc->argc == 0) {
                exit(0);
            }

            if (is_builtin(sc->argv[0])) {
                int code = execute_builtin(sc->argc, sc->argv);
                exit(code);
            } else {
                execvp(sc->argv[0], sc->argv);
                fprintf(stderr, "%s: command not found\n", sc->argv[0]);
                exit(127);
            }
        }
    }

    // Parent process: close all pipe descriptors
    if (N > 1) {
        for (int i = 0; i < N - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        free(pipes);
    }

    int last_status = 0;

    if (!pipeline->background) {
        // Wait for all child processes to complete
        for (int i = 0; i < N; i++) {
            int status = 0;
            if (waitpid(pids[i], &status, 0) > 0) {
                if (i == N - 1) {
                    if (WIFEXITED(status)) {
                        last_status = WEXITSTATUS(status);
                    } else if (WIFSIGNALED(status)) {
                        last_status = 128 + WTERMSIG(status);
                    }
                }
            }
        }
    } else {
        // Background job: print PID of the last process
        printf("[%d]\n", pids[N - 1]);
    }

    free(pids);
    return last_status;
}
