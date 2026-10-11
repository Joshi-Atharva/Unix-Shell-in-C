%{
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "executor.h"

extern int yylex(void);
void yyerror(const char *s);
int yywrap(void) { return 1; }
%}

%union {
    char *str;
    int num;
    SimpleCommand *sc;
    Pipeline *pipeline;
}

%token <str> WORD
%token NEWLINE PIPE LESS GREAT GREATGREAT TWOGREAT AMPERSAND

%type <num> opt_background
%type <sc> simple_command
%type <pipeline> pipeline

%%

input:
      /* empty */
    | input line
    ;

line:
      NEWLINE
    | pipeline opt_background NEWLINE {
        if ($1) {
            $1->background = $2;
            execute_pipeline($1);
            pipeline_free($1);
        }
    }
    | error NEWLINE {
        yyerrok;
    }
    ;

opt_background:
      /* empty */ { $$ = 0; }
    | AMPERSAND   { $$ = 1; }
    ;

pipeline:
      simple_command {
          $$ = pipeline_new();
          pipeline_add_command($$, $1);
      }
    | pipeline PIPE simple_command {
          pipeline_add_command($1, $3);
          $$ = $1;
      }
    ;

simple_command:
      WORD {
          $$ = sc_new();
          sc_add_arg($$, $1);
      }
    | LESS WORD {
          $$ = sc_new();
          sc_set_infile($$, $2);
      }
    | GREAT WORD {
          $$ = sc_new();
          sc_set_outfile($$, $2, 0);
      }
    | GREATGREAT WORD {
          $$ = sc_new();
          sc_set_outfile($$, $2, 1);
      }
    | TWOGREAT WORD {
          $$ = sc_new();
          sc_set_errfile($$, $2);
      }
    | simple_command WORD {
          sc_add_arg($1, $2);
          $$ = $1;
      }
    | simple_command LESS WORD {
          sc_set_infile($1, $3);
          $$ = $1;
      }
    | simple_command GREAT WORD {
          sc_set_outfile($1, $3, 0);
          $$ = $1;
      }
    | simple_command GREATGREAT WORD {
          sc_set_outfile($1, $3, 1);
          $$ = $1;
      }
    | simple_command TWOGREAT WORD {
          sc_set_errfile($1, $3);
          $$ = $1;
      }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "myshell syntax error: %s\n", s);
}
