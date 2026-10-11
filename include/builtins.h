#ifndef BUILTINS_H
#define BUILTINS_H

// Returns 1 if cmd is a shell builtin (cd, exit, pwd), else 0
int is_builtin(const char *cmd);

// Executes the builtin command and returns its exit status
int execute_builtin(int argc, char **argv);

#endif // BUILTINS_H
