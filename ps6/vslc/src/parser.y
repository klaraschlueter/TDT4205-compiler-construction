%{
#include <vslc.h>

#define N0C(n,t,d) do { \
    node_init ( n = malloc(sizeof(node_t)), t, d, 0 ); \
} while ( false )
#define N1C(n,t,d,a) do { \
    node_init ( n = malloc(sizeof(node_t)), t, d, 1, a ); \
} while ( false )
#define N2C(n,t,d,a,b) do { \
    node_init ( n = malloc(sizeof(node_t)), t, d, 2, a, b ); \
} while ( false )
#define N3C(n,t,d,a,b,c) do { \
    node_init ( n = malloc(sizeof(node_t)), t, d, 3, a, b, c ); \
} while ( false )

%}

%left '|'
%left '^'
%left '&'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%right '~'
%expect 1

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING

%%
program :
      global_list { N1C ( root, PROGRAM, NULL, $1 ); }
    ;
global_list :
      global { N1C ( $$, GLOBAL_LIST, NULL, $1 ); }
    | global_list global { N2C ( $$, GLOBAL_LIST, NULL, $1, $2 ); }
    ;
global:
      function { N1C ( $$, GLOBAL, NULL, $1 ); }
    | declaration { N1C ( $$, GLOBAL, NULL, $1 ); }
    ;
statement_list :
      statement { N1C ( $$, STATEMENT_LIST, NULL, $1 ); }
    | statement_list statement { N2C ( $$, STATEMENT_LIST, NULL, $1, $2 ); }
    ;
print_list :
      print_item { N1C ( $$, PRINT_LIST, NULL, $1 ); }
    | print_list ',' print_item { N2C ( $$, PRINT_LIST, NULL, $1, $3 ); }
    ;
expression_list :
      expression { N1C ( $$, EXPRESSION_LIST, NULL, $1 ); }
    | expression_list ',' expression { N2C($$, EXPRESSION_LIST, NULL, $1, $3); }
    ;
variable_list :
      identifier { N1C ( $$, VARIABLE_LIST, NULL, $1 ); }
    | variable_list ',' identifier { N2C ( $$, VARIABLE_LIST, NULL, $1, $3 ); }
    ;
argument_list :
      expression_list { N1C ( $$, ARGUMENT_LIST, NULL, $1 ); }
    | /* epsilon */ { $$ = NULL; }
    ;
parameter_list :
      variable_list { N1C ( $$, PARAMETER_LIST, NULL, $1 ); }
    | /* epsilon */ { $$ = NULL; }
    ;
declaration_list :
      declaration { N1C ( $$, DECLARATION_LIST, NULL, $1 ); }
    | declaration_list declaration { N2C ($$, DECLARATION_LIST, NULL, $1, $2); }
    ;
function :
      FUNC identifier '(' parameter_list ')' statement
        { N3C ( $$, FUNCTION, NULL, $2, $4, $6 ); }
    ;
statement :
      assignment_statement { N1C ( $$, STATEMENT, NULL, $1 ); }
    | return_statement { N1C ( $$, STATEMENT, NULL, $1 ); }
    | print_statement { N1C ( $$, STATEMENT, NULL, $1 ); }
    | if_statement { N1C ( $$, STATEMENT, NULL, $1 ); }
    | while_statement { N1C ( $$, STATEMENT, NULL, $1 ); }
    | null_statement { N1C ( $$, STATEMENT, NULL, $1 ); }
    | block { N1C ( $$, STATEMENT, NULL, $1 ); }
    ;
block :
      OPENBLOCK declaration_list statement_list CLOSEBLOCK
        { N2C ($$, BLOCK, NULL, $2, $3); }
    | OPENBLOCK statement_list CLOSEBLOCK { N1C ($$, BLOCK, NULL, $2 ); }
    ;
assignment_statement :
      identifier ':' '=' expression
        { N2C ( $$, ASSIGNMENT_STATEMENT, NULL, $1, $4 ); }
    | identifier '+' '=' expression
        { N2C ( $$, ADD_STATEMENT, NULL, $1, $4 ); }
    | identifier '-' '=' expression
        { N2C ( $$, SUBTRACT_STATEMENT, NULL, $1, $4 ); }
    | identifier '*' '=' expression
        { N2C ( $$, MULTIPLY_STATEMENT, NULL, $1, $4 ); }
    | identifier '/' '=' expression
        { N2C ( $$, DIVIDE_STATEMENT, NULL, $1, $4 ); }
    ;
return_statement :
      RETURN expression
        { N1C ( $$, RETURN_STATEMENT, NULL, $2 ); }
    ;
print_statement :
      PRINT print_list
        { N1C ( $$, PRINT_STATEMENT, NULL, $2 ); }
    ;
null_statement :
      CONTINUE
        { N0C ( $$, NULL_STATEMENT, NULL ); }
    ;
if_statement :
      IF relation THEN statement
        { N2C ( $$, IF_STATEMENT, NULL, $2, $4 ); }
    | IF relation THEN statement ELSE statement
        { N3C ( $$, IF_STATEMENT, NULL, $2, $4, $6 ); }
    ;
while_statement :
      WHILE relation DO statement
        { N2C ( $$, WHILE_STATEMENT, NULL, $2, $4 ); }
    ;
relation:
      expression '=' expression
        { N2C ( $$, RELATION, strdup("="), $1, $3 ); }
    | expression '<' expression
        { N2C ( $$, RELATION, strdup("<"), $1, $3 ); }
    | expression '>' expression
        { N2C ( $$, RELATION, strdup(">"), $1, $3 ); }
    ;
expression :
      expression '|' expression
        { N2C ( $$, EXPRESSION, strdup("|"), $1, $3 ); }
    | expression '^' expression
        { N2C ( $$, EXPRESSION, strdup("^"), $1, $3 ); }
    | expression '&' expression
        { N2C ( $$, EXPRESSION, strdup("&"), $1, $3 ); }
    | expression '+' expression
        { N2C ( $$, EXPRESSION, strdup("+"), $1, $3 ); }
    | expression '-' expression
        { N2C ( $$, EXPRESSION, strdup("-"), $1, $3 ); }
    | expression '*' expression
        { N2C ( $$, EXPRESSION, strdup("*"), $1, $3 ); }
    | expression '/' expression
        { N2C ( $$, EXPRESSION, strdup("/"), $1, $3 ); }
    | '-' expression %prec UMINUS
        { N1C ( $$, EXPRESSION, strdup("-"), $2 ); }
    | '~' expression %prec UMINUS
        { N1C ( $$, EXPRESSION, strdup("~"), $2 ); }
    | '(' expression ')' { $$ = $2; }
    | number { N1C ( $$, EXPRESSION, NULL, $1 ); }
    | identifier
        { N1C ( $$, EXPRESSION, NULL, $1 ); }
    | identifier '(' argument_list ')'
        { N2C ( $$, EXPRESSION, NULL, $1, $3 ); }
    ;
declaration :
      VAR variable_list { N1C ( $$, DECLARATION, NULL, $2 ); }
    ;
print_item :
      expression
        { N1C ( $$, PRINT_ITEM, NULL, $1 ); }
    | string
        { N1C ( $$, PRINT_ITEM, NULL, $1 ); }
    ;
identifier: IDENTIFIER { N0C($$, IDENTIFIER_DATA, strdup(yytext) ); }
number: NUMBER
      {
        int64_t *value = malloc ( sizeof(int64_t) );
        *value = strtol ( yytext, NULL, 10 );
        N0C($$, NUMBER_DATA, value );
      }
string: STRING { N0C($$, STRING_DATA, strdup(yytext) ); }
%%

int
yyerror ( const char *error )
{
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}
