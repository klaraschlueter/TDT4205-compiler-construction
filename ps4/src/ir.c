#include <vslc.h>

// Externally visible, for the generator
extern tlhash_t *global_names;
extern char **string_list;
extern size_t n_string_list, stringc;

// Implementation choices, only relevant internally
static void find_globals ( void );
/** @param function Function's symbol table entry
 *  @param root Function's root node */
static void bind_names ( symbol_t *function, node_t *root );

void
create_symbol_table ( void )
{
    /* TODO: traverse the syntax tree and create the symbol table */

    // ! Example code solely to demonstrate usage of tlhash. Make sure to remove
    // ! or comment this out when implementing your solution.

    // Initialize table
    tlhash_t *my_table = (tlhash_t*)malloc(sizeof(tlhash_t));
    tlhash_init(my_table, 64);

    char *my_key0 = "key";    // Keep in mind that these are stack allocated for simplicity, yours should not
    char *my_val0 = "valuable"; // Also, I'm using strings as values, you will be using symbol_t pointers
    char *my_key1 = "another_key";
    char *my_val1 = "more valuable";
    
    // Insert some values to the table. Remember that the length of a string
    // interpreted as an array is the string length plus one '\0' character
    tlhash_insert(my_table, my_key0, strlen(my_key0)+1, my_val0);
    tlhash_insert(my_table, my_key1, strlen(my_key1)+1, my_val1);

    // Iterate keys and lookup their values
    size_t size = tlhash_size(my_table);
    char **keys = (char **)malloc(size);
    tlhash_keys(my_table, keys);
    char *val;
    for (int i = 0; i < size; i++) {
        tlhash_lookup(my_table, keys[i], strlen(keys[i])+1, &val);
        printf("my_table[%s] = \"%s\"\n", keys[i], val);
    }

    // Free allocated memory when done with the symbol table
    tlhash_finalize(my_table);
    free(my_table);
    free(keys);
}


void
print_symbol_table ( void )
{
    /* TODO: output its contents */
}

void
destroy_symbol_table ( void )
{
    /* TODO: release memory allocated to the symbol table */
}

void
find_globals ( void )
{
    /* TODO: Populate symbol table with global variables and functions */
}


void 
bind_names ( symbol_t *function, node_t *root )
{
    /* TODO: Bind names and string literals in local symbol table */
}
