#include <vslc.h>

/**Generate table of strings in a rodata section. */
void generate_stringtable ( void );
/**Declare global variables in a bss section */
void generate_global_variables ( void );
/**Generate function entry code
 * @param function symbol table entry of function */
void generate_function ( symbol_t *function );
/**Generate code for a node in the AST, to be called recursively from
 * generate_function
 * @param node root node of current code block */
static void generate_node ( node_t *node );
/**Initializes program (already implemented) */
void generate_main ( symbol_t *first );

#define MIN(a,b) (((a)<(b)) ? (a):(b))

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

void
generate_program ( void )
{
    /* TODO: Emit assembly instructions for functions, function calls,
     * print statements and expressions.
     * The provided function 'generate_main' creates a program entry point
     * for the function symbol it is given as argument.
     */

    
    symbol_t* globals[global_names->size];
    tlhash_values(global_names, (void**) &globals);

    // TODO: Implement
    // - Generate string table
    generate_stringtable();
    // - Declare global variables
    generate_global_variables();

    
    puts(".text");
    // - Generate code for all functions
    
    for (size_t i = 0; i < global_names->size; i++)
    {
        if (globals[i]->type == SYM_FUNCTION) {
            generate_function(globals[i]);
        }
    }
    
    // - Generate main (function already implemented) by assigning either the
    //   function named main or the first function of the source file if no 
    //   main exists.
    
    symbol_t* main = NULL;
    if (tlhash_lookup(global_names, "main", 4, (void**) &main) != TLHASH_SUCCESS)
    {
        // Assign the function appearing first in our program:
        for (size_t i = 0; i < global_names->size; i++)
        {
            if (globals[i]->type == SYM_FUNCTION && globals[i]->seq == 0) {
                main = globals[i];
            }
        }
    }
    generate_main(main);
}

void
generate_stringtable ( void )
{
	/* These can be used to emit numbers, strings and a run-time
	 * error msg. from main
	 */
	puts(".data");
	puts(".intout:\t.asciz \"\%ld \"");
	puts(".strout:\t.asciz \"\%s \"");
	puts(".errout:\t.asciz \"Wrong number of arguments\"");

    for (size_t i = 0; i < stringc; i++)
    {
        printf(".str%04zu:\t.asciz %s\n", i, string_list[i]);
    }
}

void
generate_global_variables ( void )
{
    puts(".bss");
    puts(".align 8");
    symbol_t* globals[global_names->size];
    tlhash_values(global_names, (void**) &globals);
    for (size_t i = 0; i < global_names->size; i++)
    {
        if (globals[i]->type == SYM_GLOBAL_VAR) {
            printf(".%s:\n", globals[i]->name);
        }
    }
}

void
generate_function ( symbol_t *function )
{
    size_t num_params = function->nparms;
    size_t num_locals_vars = (function->locals->size - num_params);

    printf(".globl _%s\n", function->name);
    printf("_%s:\n", function->name);
    // Function prologue -> setup stack frame
    puts("\tpushq   %rbp");
    puts("\tmovq    %rsp, %rbp");

    // save aside parameter values passed in the registers (the rest [if any] come served on the stack already)
    for (int i = 0; i < MIN(num_params, 6); i++) 
    {
        printf("\tpushq %s\n", record[i]);
    }
    
    // allocate space for local variables
    if (num_locals_vars > 0)
        printf("\tsubq $%zu, %%rsp\n", 8 * num_locals_vars);
    
    // ensure stack size is 16byte-aligned
    if (function->locals->size % 2 == 1)
        puts("\tpushq $0");

    // Function epilogue -> tear down stack frame
    puts("\tleave");
    puts("\tret");
}

void
generate_node ( node_t *node )
{
    // TODO: Generate code corresponding to node
}

/**Generates the main function with argument parsing and calling of our
 * main function (first, if no function is named main)
 * @param first Symbol table entry of our main function */
void
generate_main ( symbol_t *first )
{
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );

    //prologue
    puts ( "\tpushq   %rbp" );
    puts ( "\tmovq    %rsp, %rbp" );

    // if main is called with the wrong number of arguments, abort and print error message, assumes first argument holds number of arguments?
    printf ( "\tsubq\t$1,%%rdi\n" );
    printf ( "\tcmpq\t$%zu,%%rdi\n", first->nparms );
    printf ( "\tjne\tABORT\n" );        

    // if main is called with no arguments (other than number of arguments?), skip past the argument parsing
    printf ( "\tcmpq\t$0,%%rdi\n" );    
    printf ( "\tjz\tSKIP_ARGS\n" );


    printf ( "\tmovq\t%%rdi,%%rcx\n" );
    printf ( "\taddq $%zu, %%rsi\n", 8*first->nparms );
    printf ( "PARSE_ARGV:\n" );
    printf ( "\tpushq %%rcx\n" );
    printf ( "\tpushq %%rsi\n" );

    printf ( "\tmovq\t(%%rsi),%%rdi\n" );
    printf ( "\tmovq\t$0,%%rsi\n" );
    printf ( "\tmovq\t$10,%%rdx\n" );
    printf ( "\tcall\tstrtol\n" );

/*  Now a new argument is an integer in rax */

    printf ( "\tpopq %%rsi\n" );
    printf ( "\tpopq %%rcx\n" );
    printf ( "\tpushq %%rax\n" );
    printf ( "\tsubq $8, %%rsi\n" );
    printf ( "\tloop PARSE_ARGV\n" );

    /* Now the arguments are in order on stack */
    for ( int arg=0; arg<MIN(6,first->nparms); arg++ )
        printf ( "\tpopq\t%s\n", record[arg] );

    printf ( "SKIP_ARGS:\n" );
    printf ( "\tcall\t_%s\n", first->name );
    printf ( "\tjmp\tEND\n" );
    printf ( "ABORT:\n" );
    printf ( "\tmovq\t$.errout, %%rdi\n" );
    printf ( "\tcall puts\n" );

    printf ( "END:\n" );
    puts ( "\tmovq    %rax, %rdi" );
    puts ( "\tcall    exit" );
}
