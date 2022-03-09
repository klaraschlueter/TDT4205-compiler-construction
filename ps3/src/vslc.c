#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <vslc.h>


/* Global state */

node_t *root;               // Syntax tree


/* Command line option parsing for the main function */
static void options ( int argc, char **argv );
bool
    print_full_tree = false,
    print_simplified_tree = false,
    new_print_style = true;


/* Entry point */
int
main ( int argc, char **argv )
{
    options ( argc, argv );

    yyparse();  // Generated from grammar/bison, constructs syntax tree

    if ( print_full_tree )
        print_syntax_tree ();
    simplify_syntax_tree ();    // In tree.c
    if ( print_simplified_tree )
        print_syntax_tree ();

    destroy_syntax_tree ();     // In tree.c
}


static const char *usage =
"Command line options\n"
"\t-h\tOutput this text and halt\n"
"\t-t\tOutput the full syntax tree\n"
"\t-T\tOutput the simplified syntax tree\n"
"\t-u\tDo not use print style more like the tree command\n";


static void
options ( int argc, char **argv )
{
    int o;
    while ( (o=getopt(argc,argv,"htTu")) != -1 )
    {
        switch ( o )
        {
            case 'h':
                printf ( "%s:\n%s", argv[0], usage );
                exit ( EXIT_FAILURE );
                break;
            case 't':   print_full_tree = true;             break;
            case 'T':   print_simplified_tree = true;       break;
            case 'u':   new_print_style = false;            break;
        }
    }
}

