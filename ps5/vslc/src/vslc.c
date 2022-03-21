#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <vslc.h>


/* Global state */

node_t *root;               // Syntax tree
tlhash_t *global_names;     // Symbol table
char **string_list;         // List of strings in the source
size_t n_string_list = 8;   // Initial string list capacity (grow on demand)
size_t stringc = 0;         // Initial string count

/* Command line option parsing for the main function */
static void options ( int argc, char **argv );
bool
    print_full_tree = false,
    print_simplified_tree = false,
    print_symbol_table_contents = false,
    print_generated_program = true,
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

    create_symbol_table ();   // In ir.c
    if ( print_symbol_table_contents )
        print_symbol_table();

    if ( print_generated_program )
        generate_program ();    // In generator.c


    destroy_syntax_tree ();     // In tree.c
    destroy_symbol_table ();    // In ir.c
}


static const char *usage =
"Command line options\n"
"\t-h\tOutput this text and halt\n"
"\t-t\tOutput the full syntax tree\n"
"\t-T\tOutput the simplified syntax tree\n"
"\t-s\tOutput the symbol table contents\n"
"\t-q\tQuiet: suppress output from the code generator\n"
"\t-u\tDo not use print style more like the tree command\n";


static void
options ( int argc, char **argv )
{
    int o;
    while ( (o=getopt(argc,argv,"htTsqu")) != -1 )
    {
        switch ( o )
        {
            case 'h':
                printf ( "%s:\n%s", argv[0], usage );
                exit ( EXIT_FAILURE );
                break;
            case 't':   print_full_tree = true;             break;
            case 'T':   print_simplified_tree = true;       break;
            case 's':   print_symbol_table_contents = true; break;
            case 'q':   print_generated_program = false;    break;
            case 'u':   new_print_style = false;            break;
        }
    }
}

