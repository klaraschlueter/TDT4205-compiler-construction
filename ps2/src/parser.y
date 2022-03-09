%{
#include <vslc.h>
%}

%start expression

%token FUNC PRINT RETURN CONTINUE IF THEN ELSE WHILE DO OPENBLOCK CLOSEBLOCK
%token VAR NUMBER IDENTIFIER STRING

%%
number : NUMBER {    
    int64_t *value = malloc ( sizeof(int64_t) );
    *value = strtol ( yytext, NULL, 10 );
    
    node_init($$ = (node_t*)malloc(sizeof(node_t)), NUMBER_DATA, value, 0);
//  root = $$;  
}
    ;
expression : NUMBER '+' NUMBER {
    char *value = (char*)malloc ( 1 );
    *value = '+';
    node_init($$ = (node_t*)malloc(sizeof(node_t)), EXPRESSION, value, 2, $1, $2);
    root = $$;
    
    };
    
    // add a list construct

program : 
      dummy0 { root = $$; }
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
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}
