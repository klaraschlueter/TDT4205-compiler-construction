#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <vslc.h>


/* Global state */

extern node_t *root;               // Syntax tree


/* Command line option parsing for the main function */
static void options ( int argc, char **argv );
bool
    print_full_tree = true;


/* Entry point */
int
main ( int argc, char **argv )
{
    options ( argc, argv );
    //root = (node_t*)malloc(sizeof(node_t));
    //int64_t value = 123;
    //int64_t *pointer;
    //pointer = &value;
    
    // Here's the problem:	
    /*node_init(root, NUMBER_DATA, pointer, 0);
    print_syntax_tree();
    
    destroy_syntax_tree();
    print_syntax_tree();
    */
    
    yyparse();  // Generated from grammar/bison, constructs syntax tree

    if ( print_full_tree )
        print_syntax_tree ();

    destroy_syntax_tree ();     // In tree.c
    root = NULL;
    
    print_syntax_tree ();
}


static const char *usage =
"Command line options\n"
"\t-h\tOutput this text and halt\n"
"\t-t\tOutput the full syntax tree\n";


static void
options ( int argc, char **argv )
{
    int o;
    while ( (o=getopt(argc,argv,"ht")) != -1 )
    {
        switch ( o )
        {
            case 'h':
                printf ( "%s:\n%s", argv[0], usage );
                exit ( EXIT_FAILURE );
                break;
            case 't':   print_full_tree = true;             break;
        }
    }
}

