%{
#include <vslc.h>
// enum { FUNC, OPENBLOCK, CLOSEBLOCK, RETURN, PRINT, CONTINUE, IF, THEN, ELSE, WHILE, DO, VAR, INSTRING, FUNC_NAME, PARAM_NAME, END_PARAMS };
%}
%option noyywrap
%option array
%option yylineno

WHITESPACE [\ \t\v\r\n]
COMMENT \/\/[^\n]+
NAME [A-Za-z][A-Za-z0-9]*
NUMBER [1-9][0-9]*
%%
<INITIAL>{WHITESPACE}+  { /* Eliminate whitespace */ }
{COMMENT}               { /* Eliminate comments */ }
{NUMBER}                { return NUMBER; }
var                     { return VAR; }
{NAME}                  { return IDENTIFIER; }
.                       { return yytext[0]; }
%%
/* func                                    { BEGIN(EXP_FUNC); }
<EXP_FUNC>{NAME}                        { BEGIN(FUNC); return FUNC_NAME; }
<FUNC>\(                                { BEGIN(EXP_PARAM_NAME); }
<EXP_PARAM_NAME>{NAME}                  { BEGIN(EXP_PARAM_END); return PARAM_NAME; }
<EXP_PARAM_NAME>\)                      { BEGIN(INITIAL); }
<EXP_PARAM_END>\)                       { BEGIN(INITIAL); }
<EXP_PARAM_END>,                        { BEGIN(EXP_PARAM_NAME); }
begin                                   { return OPENBLOCK; }
end                                     { return CLOSEBLOCK; }
return                                  { return RETURN; }
print                                   { return PRINT; }
continue                                { return CONTINUE; }
if                                      { return IF; }
then                                    { return THEN; }
else                                    { return ELSE; }
while                                   { return WHILE; }
do                                      { return DO; }
var                                     { return VAR; }
<INITIAL>\"                             { BEGIN(STRING); }
<STRING>\"                              { BEGIN(INITIAL); }
<STRING>.                               { return INSTRING; }
*/

/*int main (void) {
    int token = 0;
    while (token != CLOSEBLOCK) {
        token = yylex();
        switch (token) {
            case FUNC_NAME: printf ("Found func, name: %s \n", yytext); break;
            case PARAM_NAME: printf ("    param: %s\n", yytext); break;
            case OPENBLOCK: printf ("Found begin\n"); break;
            case CLOSEBLOCK: printf ("Found end\n"); break;
            case RETURN: printf ("Found return\n"); break;
            case PRINT: printf ("Found print\n"); break;
            case CONTINUE: printf ("Found continue\n"); break;
            case IF: printf ("Found if\n"); break;
            case THEN: printf ("Found then\n"); break;
            case ELSE: printf ("Found else\n"); break;
            case WHILE: printf ("Found while\n"); break;
            case DO: printf ("Found do\n"); break;
            case VAR: printf ("Found var\n"); break;
            case INSTRING: printf ("%s", yytext); break;
//            case ENDSTRING: print ("\n"
            default: printf ("Found something else: %s\n", yytext); break;
        }
    }
}*/
