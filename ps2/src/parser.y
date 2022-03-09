%{
#include <vslc.h>
%}

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING

/* %left transform 1 + 2 + 3 into (1 + 2) + 3
   more on left and nonassoc:
      https://iqcode.com/code/shell/nonassoc-in-yacc
*/ 

%left '+' '-'     
%left '*' '/'     
%nonassoc UMINUS
%nonassoc IF THEN
%nonassoc ELSE

/*
    If encountering segmentation faults implementing new productions:
    1. make sure to set root = $$
*/
%%

program
: global_list {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), PROGRAM, NULL, 1, $1);
    root = $$;
  }
;

global_list
: global {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), GLOBAL_LIST, NULL, 1, $1);
  }
| global_list global {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), GLOBAL_LIST, NULL, 2, $1, $2);
  }
;

global 
: declaration {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), GLOBAL, NULL, 1, $1);
  }
; /*TODO: ADD function before declaration as another match of global*/

declaration
: VAR variable_list {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), DECLARATION, NULL, 1, $2);
  }
;

variable_list
: identifier {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), VARIABLE_LIST, NULL, 1, $1);
  }
| variable_list ',' identifier {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), VARIABLE_LIST, NULL, 2, $1, $3);
  }
;

identifier
: IDENTIFIER {
    char* value = strdup(yytext);
    node_init($$ = (node_t*)malloc(sizeof(node_t)), IDENTIFIER_DATA, value, 0);
  }
;

expression_list
: expression {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), EXPRESSION_LIST, NULL, 1, $1);
}
| expression_list ',' expression {
    node_init($$ = (node_t*)malloc(sizeof(node_t)), EXPRESSION_LIST, NULL, 2, $1, $3);
}
;

expression 
: number '+' number {
    char* value = (char*)malloc(sizeof(char));
    *value = '+';
    node_init($$ = (node_t*)malloc(sizeof(node_t)), EXPRESSION, value, 2, $1, $3);
  }
;

number 
: NUMBER {    
    int64_t* value = (int64_t*)malloc(sizeof(int64_t));
    *value = strtol ( yytext, NULL, 10 );
    node_init($$ = (node_t*)malloc(sizeof(node_t)), NUMBER_DATA, value, 0);
  }
;

    
   
dummy0 :
      dummy1 { $$ = $1; }
    | dummy2 { $$ = $1; }
    ;
dummy1 :
      RETURN NUMBER 
      { 
        node_init($$ = malloc(sizeof(node_t)), STATEMENT, NULL, 1, $2); // 2: NUMBER 
      }
    ;
dummy2 :
      VAR IDENTIFIER '+''=' NUMBER 
      {
        node_init($$ = malloc(sizeof(node_t)), STATEMENT, NULL, 2, $2, $5); // 2: IDENTIFIER, 5: NUMBER
      }
    | /* epsilon */ { return NULL; }
    ;

%%

int
yyerror ( const char *error )
{
    fprintf ( stderr, "%s on line %d\nEncountered: %s\n", error, yylineno, yytext );
    exit ( EXIT_FAILURE );
}
