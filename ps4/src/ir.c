#include <vslc.h>

// Externally visible, for the generator
extern node_t* root;
extern tlhash_t* global_names;
extern char** string_list;
extern size_t n_string_list, stringc;

// Implementation choices, only relevant internally
static void find_globals ( void );
/** @param function Function's symbol table entry
 *  @param root Function's root node */
static void bind_names ( symbol_t *function, node_t *root );

void
create_symbol_table ( void )
{
    global_names = (tlhash_t*)malloc(sizeof(tlhash_t));
    tlhash_init(global_names, 64);  // TODO: use the global list length for bucket size instead?

    find_globals(); 

    /* TODO: traverse the syntax tree and create the symbol table */
    
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
    // We start at 1, allowing us to use 0 as indicator for an undefined sequence number
    size_t sequence_number = 1;

    if (root->n_children > 1)
    {
        printf("Program has multiple children in AST, should only have one!\n");
        abort();
    }

    if (root->n_children == 0)
    {
        return;
    }

    
    node_t* globals = root->children[0];
    if (globals->type != GLOBAL_LIST)
    {
        printf("Top level is not global list. Invalid Program!\n");
        abort();
    }

    for (uint64_t i = 0; i < globals->n_children; i++)
    {
        node_t* child = globals->children[i];
        switch (child->type)
        {
        case DECLARATION:;
            node_t* var_list = child->children[0];
            for (uint64_t j = 0; j < var_list->n_children; j++)
            {
                node_t* current_node = var_list->children[j];
                char* key = current_node->data;

                symbol_t* value = (symbol_t*)malloc(sizeof(symbol_t));
                *value = (symbol_t) {
                    .type = SYM_GLOBAL_VAR,
                    .node = current_node,   // Assuming we want to attach the symbol to the node that represents the variable, 
                                            // and not the node that represents the variable list
                    .seq = 0,               // Assuming we don't need this to be sequenced
                    .nparams = 0,
                    .locals = NULL
                };
                tlhash_insert(global_names, key, strlen(key) + 1, value);
            }
            break;
        case FUNCTION:;
            char* key = child->children[0]->data;
            node_t* parmeter_list = child->children[1];
            size_t nparams = (parmeter_list == NULL) ? 0 : parmeter_list->n_children;
            tlhash_t* local_symbols = (tlhash_t*)malloc(sizeof(tlhash_t));

            symbol_t* value = (symbol_t*)malloc(sizeof(symbol_t));
            *value = (symbol_t) {
                    .type = SYM_FUNCTION,
                    .node = child,
                    .seq = sequence_number++,               // Assuming we don't need this to be sequenced
                    .nparams = nparams,
                    .locals = local_symbols
                };
                tlhash_insert(global_names, key, strlen(key) + 1, value);
            break;
        default:
            printf("Global list contains a node of type %s, only declarations and functions are allowed!\n", node_string[child->type]);
            abort();
            break;
        }
    }
    


    /* TODO: Populate symbol table with global variables and functions */
}


void 
bind_names ( symbol_t *function, node_t *root )
{
    /* TODO: Bind names and string literals in local symbol table */
}
