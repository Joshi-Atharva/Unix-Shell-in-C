#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command.h"

// Executes a pipeline of commands with proper pipes, redirection, and job waiting.
// Returns the exit code of the last command in the pipeline.
int execute_pipeline(Pipeline *pipeline);

#endif // EXECUTOR_H
