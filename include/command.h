#ifndef COMMAND_H
#define COMMAND_H

// Represents a single command stage (e.g., "grep -i foo") with its redirections
typedef struct SimpleCommand {
    char **argv;        // NULL-terminated array of arguments for execvp
    int argc;           // Argument count
    int capacity;       // Allocated capacity of argv

    char *infile;       // < input_file (NULL if none)
    char *outfile;      // > or >> output_file (NULL if none)
    int append_out;     // 0 for '>', 1 for '>>'
    char *errfile;      // 2> error_file (NULL if none)
} SimpleCommand;

// Represents a full pipeline of simple commands (e.g., cmd1 | cmd2 | cmd3)
typedef struct Pipeline {
    SimpleCommand **commands; // Array of simple command pointers
    int cmd_count;            // Number of command stages
    int capacity;             // Allocated capacity of commands array

    int background;           // 1 if terminated with '&', else 0
} Pipeline;

// SimpleCommand management
SimpleCommand* sc_new(void);
void sc_add_arg(SimpleCommand *sc, char *arg);
void sc_set_infile(SimpleCommand *sc, char *infile);
void sc_set_outfile(SimpleCommand *sc, char *outfile, int append);
void sc_set_errfile(SimpleCommand *sc, char *errfile);
void sc_free(SimpleCommand *sc);

// Pipeline management
Pipeline* pipeline_new(void);
void pipeline_add_command(Pipeline *p, SimpleCommand *sc);
void pipeline_free(Pipeline *p);

#endif // COMMAND_H
