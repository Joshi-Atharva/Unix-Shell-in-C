%{
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "parser.tab.h"
%}

%option noyywrap
%option nounput
%option noinput

%%

[ \t]+                  { /* Discard spaces and tabs */ }
"\n"                    { return NEWLINE; }
"|"                     { return PIPE; }
"<"                     { return LESS; }
">>"                    { return GREATGREAT; }
">"                     { return GREAT; }
"2>"                    { return TWOGREAT; }
"&"                     { return AMPERSAND; }

    /* Double-quoted string with basic escape handling */
\"([^"\\]|\\.)*\"       {
    size_t len = strlen(yytext);
    char *str = (char*)malloc(len);
    size_t out = 0;
    for (size_t i = 1; i < len - 1; i++) {
        if (yytext[i] == '\\' && i + 1 < len - 1) {
            i++;
        }
        str[out++] = yytext[i];
    }
    str[out] = '\0';
    yylval.str = str;
    return WORD;
}

    /* Single-quoted string literal */
'[^']*'                 {
    size_t len = strlen(yytext);
    char *str = (char*)malloc(len);
    memcpy(str, yytext + 1, len - 2);
    str[len - 2] = '\0';
    yylval.str = str;
    return WORD;
}

    /* Environment variable expansion: $VAR */
\$[a-zA-Z_][a-zA-Z0-9_]* {
    char *val = getenv(yytext + 1);
    yylval.str = strdup(val ? val : "");
    return WORD;
}

    /* Standard word token */
[^ \t\n|<>&"']+         {
    yylval.str = strdup(yytext);
    return WORD;
}

.                       { /* Ignore stray unexpected characters */ }

%%
