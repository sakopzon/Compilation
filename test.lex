%{
#include <stdio.h>
void showToken(char *);
%}

%option yylineno noyywrap
%option   outfile="flex_example1.c" header-file="flex_example1.h"

DIGIT       ([0-9])
CHAR        ([a-zA-Z])
whitespace  ([\t\n ])
NUM         ({DIGIT}+)
ID          ({CHAR}[{CHAR}{NUM}]*)
STRING      ()

%%
{NUM}                       showToken("num");
{ID}                        showToken("id");
{STRING}                    showToken("str");
{whitespace}                ;
.                           printf("lex fails to recognize this (%s)!\n", yytext);
%%

void showToken(char *name)
{
    printf("<%s,%s>\n", name, yytext);
}

