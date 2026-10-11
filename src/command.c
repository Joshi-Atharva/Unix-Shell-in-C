#include <stdlib.h>
#include <string.h>
#include "command.h"

SimpleCommand* sc_new(void) {
    SimpleCommand *sc = (SimpleCommand*)malloc(sizeof(SimpleCommand));
    if (!sc) return NULL;

    sc->capacity = 4;
    sc->argc = 0;
    sc->argv = (char**)malloc(sizeof(char*) * sc->capacity);
    if (sc->argv) {
        sc->argv[0] = NULL;
    }
    sc->infile = NULL;
    sc->outfile = NULL;
    sc->append_out = 0;
    sc->errfile = NULL;
    return sc;
}

void sc_add_arg(SimpleCommand *sc, char *arg) {
    if (!sc || !arg) return;

    if (sc->argc + 2 > sc->capacity) {
        sc->capacity *= 2;
        char **new_argv = (char**)realloc(sc->argv, sizeof(char*) * sc->capacity);
        if (!new_argv) return;
        sc->argv = new_argv;
    }

    sc->argv[sc->argc++] = arg;
    sc->argv[sc->argc] = NULL;
}

void sc_set_infile(SimpleCommand *sc, char *infile) {
    if (!sc) return;
    if (sc->infile) {
        free(sc->infile);
    }
    sc->infile = infile;
}

void sc_set_outfile(SimpleCommand *sc, char *outfile, int append) {
    if (!sc) return;
    if (sc->outfile) {
        free(sc->outfile);
    }
    sc->outfile = outfile;
    sc->append_out = append;
}

void sc_set_errfile(SimpleCommand *sc, char *errfile) {
    if (!sc) return;
    if (sc->errfile) {
        free(sc->errfile);
    }
    sc->errfile = errfile;
}

void sc_free(SimpleCommand *sc) {
    if (!sc) return;

    if (sc->argv) {
        for (int i = 0; i < sc->argc; i++) {
            if (sc->argv[i]) {
                free(sc->argv[i]);
            }
        }
        free(sc->argv);
    }

    if (sc->infile) free(sc->infile);
    if (sc->outfile) free(sc->outfile);
    if (sc->errfile) free(sc->errfile);
    free(sc);
}

Pipeline* pipeline_new(void) {
    Pipeline *p = (Pipeline*)malloc(sizeof(Pipeline));
    if (!p) return NULL;

    p->capacity = 4;
    p->cmd_count = 0;
    p->commands = (SimpleCommand**)malloc(sizeof(SimpleCommand*) * p->capacity);
    p->background = 0;
    return p;
}

void pipeline_add_command(Pipeline *p, SimpleCommand *sc) {
    if (!p || !sc) return;

    if (p->cmd_count + 1 > p->capacity) {
        p->capacity *= 2;
        SimpleCommand **new_cmds = (SimpleCommand**)realloc(p->commands, sizeof(SimpleCommand*) * p->capacity);
        if (!new_cmds) return;
        p->commands = new_cmds;
    }

    p->commands[p->cmd_count++] = sc;
}

void pipeline_free(Pipeline *p) {
    if (!p) return;

    if (p->commands) {
        for (int i = 0; i < p->cmd_count; i++) {
            sc_free(p->commands[i]);
        }
        free(p->commands);
    }
    free(p);
}
