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
static void bind_names ( symbol_t *function );
static void print_symbol( symbol_t* symbol );

// We start at 1, allowing us to use 0 as indicator for an undefined sequence number
size_t sequence_number = 1;

void 
print_symbol(symbol_t* symbol) 
{
    char* symbol_type_names[] = {
        "SYM_GLOBAL_VAR", "SYM_FUNCTION", 
        "SYM_PARAMETER", "SYM_LOCAL_VAR"
    };
    printf("Name:               %s\n", symbol->name);
    printf("Type:               %s\n", symbol_type_names[symbol->type]);
    printf("Bound Node:         %p\n", symbol->node);
    printf("Sequence number:    %zu\n", symbol->seq);
    printf("#params:            %zu\n", symbol->nparams);
    printf("Locals:             %p\n", symbol->locals);
}

void
create_symbol_table ( void )
{
    global_names = (tlhash_t*)malloc(sizeof(tlhash_t));
    tlhash_init(global_names, 256);  // TODO: use the global list length for bucket size instead?

    find_globals(); 

    /* TODO: traverse the syntax tree and create the symbol table */
    /*
        nodes we need to react to:
        Statement
    */
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
check_node_type (node_t* node, node_index_t expected_type) {
    if (node == NULL)
    {
        printf("Node was null! Expected a node of type %s\n",
                    node_string[expected_type]);
        abort();
    }
    if (node->type != expected_type)
    {
        printf("Wrong node type! Expected %s, got %s\n",
                    node_string[expected_type], node_string[node->type]);
        abort();
    }
}

void
check_nchildren (node_t* node, uint64_t expected_nchildren)
{
    if (node->n_children != expected_nchildren)
    {
        printf("Wrong number of children! Expected %ld, got %ld\n",
                    expected_nchildren, node->n_children);
        abort();
    }
}

void declaration_into_sym_table (node_t* declaration_node, tlhash_t* symbol_table)
{
    check_node_type(declaration_node, DECLARATION);
    check_nchildren(declaration_node, 1);
    
    node_t* var_list = declaration_node->children[0];
    for (uint64_t i = 0; i < var_list->n_children; i++)
    {
        node_t* current_node = var_list->children[i];
        char* key = current_node->data;

        symbol_t* value = (symbol_t*)malloc(sizeof(symbol_t));
        *value = (symbol_t) {
            .name = key,
            .type = SYM_GLOBAL_VAR,
            .node = current_node,   // Assuming we want to attach the symbol to the node that represents the variable, 
                                    // and not the node that represents the variable list
            .seq = 0,               // Assuming we don't need this to be sequenced
            .nparams = 0,
            .locals = NULL
        };
        tlhash_insert(symbol_table, key, strlen(key) + 1, value);
    }
}

void fill_parameters_into_func_table(symbol_t* function)
{
    node_t* node = function->node;
    check_node_type(node, FUNCTION);
    check_nchildren(node, 3);

    node_t* parameters = node->children[1];
    if (parameters != NULL) 
    {
        check_node_type(parameters, VARIABLE_LIST);
        if (function->nparams != parameters->n_children)
        {
            printf("Number of parameters specified in given symbol table does not equal number of parameters in AST.\n");
            abort();
        }
        // Add all the parameters to function's locals
        for (int i = 0; i < function->nparams; i++)
        {
            node_t* param = parameters->children[i];
            check_node_type(param, IDENTIFIER_DATA);
            char* key = param->data;
    
            // make symbol for parameter i
            symbol_t* value = (symbol_t*)malloc(sizeof(symbol_t));
            *value = (symbol_t) {
                .name = key,
                .type = SYM_PARAMETER,
                .node = param,
                .seq = sequence_number++,
                .nparams = 0,
                .locals = NULL
            };
            tlhash_insert( function->locals, key, strlen(key) + 1, value);
        }
    }
}

void
find_globals ( void )
{
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
            declaration_into_sym_table(child, global_names);
            break;
        case FUNCTION:;
            char* key = child->children[0]->data;
            node_t* parmeter_list = child->children[1];
            size_t nparams = (parmeter_list == NULL) ? 0 : parmeter_list->n_children;
            tlhash_t* local_symbols = (tlhash_t*)malloc(sizeof(tlhash_t));
            if( tlhash_init(local_symbols, 64) != TLHASH_SUCCESS )
            {
                printf("Failed initializing local symbol table!\n");
            }
            symbol_t* value = (symbol_t*)malloc(sizeof(symbol_t));
            *value = (symbol_t) {
                .name = key,
                .type = SYM_FUNCTION,
                .node = child,
                .seq = sequence_number++,      
                .nparams = nparams,
                .locals = local_symbols
            };
            tlhash_insert(global_names, key, strlen(key) + 1, value);

            fill_parameters_into_func_table ( value );

            // TODO: Multi-blocked function body may contain multiple scopes

            bind_names(value);

            break;
        default:
            printf("Global list contains a node of type %s, only declarations and functions are allowed!\n", node_string[child->type]);
            abort();
            break;
        }
    }
}

void bind_subtree_names ( tlhash_t* local_names, node_t* root)
{
    if (root == NULL)
    {
        return;
    }
    
    switch (root->type)
    {
    case DECLARATION:;
        //fill table
        declaration_into_sym_table(root, local_names);
        return;
        break;
        
    case BLOCK:;
        /* code */
        break;
    
    case IDENTIFIER_DATA:
        if (tlhash_lookup(local_names, root->data, strlen(root->data) + 1, (void**) &root->entry) != TLHASH_SUCCESS) {
            // The identifier was not found in local names, try global names:
            if (tlhash_lookup(global_names, root->data, strlen(root->data) + 1, (void**) &root->entry) != TLHASH_SUCCESS) {
                // TODO: elegantly fail
                printf("Identifier \"%s\" not found in local or global scope!\n", (char*)root->data);
                abort();
            }
        }        
        break;

    default:
        break;
    }

    if (root->type == BLOCK)
    {
        // TODO NEXT introduce a new scope (a new symbol table)
        // How would we "attach" it?
    }
    if (root->n_children > 0)
    {
        // recursive call for all children
        for (uint64_t i = 0; i < root->n_children; i++)
        {
            bind_subtree_names(local_names, root->children[i]);
        }      
    }
}

void 
bind_names ( symbol_t* function )
{
    node_t* function_body = function->node->children[2];
    bind_subtree_names (function->locals, function_body);
    
    /* TODO: Bind names and string literals in local symbol table */
    // TODO: why are String literals named separately?
}