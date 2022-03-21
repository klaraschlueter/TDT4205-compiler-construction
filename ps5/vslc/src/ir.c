#include <vslc.h>

// Externally visible, for the generator
extern tlhash_t *global_names;
extern char **string_list;
extern size_t n_string_list, stringc;


// Implementation choices, only relevant internally
static void find_globals ( void );
static void bind_names ( symbol_t *function, node_t *root );
static void print_symbols ( tlhash_t *table );
static void destroy_symtab ( void );

// Internal details of name resolution
static size_t n_scopes = 1, scope_depth = 0;
static tlhash_t **scopes = NULL;

/* External interface */

void
create_symbol_table ( void )
{
    find_globals();
    size_t n_globals = tlhash_size ( global_names );
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t i=0; i<n_globals; i++ )
        if ( global_list[i]->type == SYM_FUNCTION )
            bind_names ( global_list[i], global_list[i]->node );
}


void
print_symbol_table ( void )
{
    print_symbols ( global_names );
}


void
destroy_symbol_table ( void )
{
    destroy_symtab();
}

/* Internal matters */


static void
print_symbols ( tlhash_t *table )
{
    if ( table == NULL )
        return;
    size_t n_entries = tlhash_size(table);
    symbol_t *entry_list[n_entries];
    tlhash_values ( table, (void **)&entry_list );
    for ( size_t e=0; e<n_entries; e++ )
    {
        switch ( entry_list[e]->type )
        {
            case SYM_FUNCTION:
                fprintf ( stderr, "function: %s\n", entry_list[e]->name );
                if ( entry_list[e]->type == SYM_FUNCTION )
                    print_symbols ( entry_list[e]->locals );
                break;
            case SYM_GLOBAL_VAR:
                fprintf ( stderr, "global var: %s\n", entry_list[e]->name );
                break;
            case SYM_PARAMETER:
                fprintf ( stderr, "parameter: %s\n", entry_list[e]->name );
                break;
            case SYM_LOCAL_VAR:
                fprintf ( stderr, "local var: %s\n", entry_list[e]->name );
                break;
            default:
                /* This should never happen if all symbols have correct type */
                fprintf ( stderr,
                    "** Unknown symbol: %s\n", entry_list[e]->name
                );
                break;
        }
    }
}


static void
add_global ( symbol_t *symbol )
{
    tlhash_insert (
        global_names, symbol->name, strlen(symbol->name), symbol
    );
}


static void
find_globals ( void )
{
    global_names = malloc ( sizeof(tlhash_t) );
    tlhash_init ( global_names, 32 );
    string_list = malloc ( n_string_list * sizeof(char * ) );
    size_t n_functions = 0;

    node_t *global_list = root->children[0];
    for ( uint64_t g=0; g<global_list->n_children; g++ )
    {
        node_t *global = global_list->children[g], *namelist;
        symbol_t *symbol;
        switch ( global->type )
        {
            case FUNCTION:
                symbol = malloc ( sizeof(symbol_t) );
                *symbol = (symbol_t) {
                    .type = SYM_FUNCTION,
                    .name = global->children[0]->data,
                    .node = global->children[2],
                    .seq = n_functions,
                    .nparms = 0,
                    .locals = malloc ( sizeof(tlhash_t) )
                };
                n_functions++;

                tlhash_init ( symbol->locals, 32 );
                if ( global->children[1] != NULL )
                {
                    symbol->nparms = global->children[1]->n_children;
                    for ( int p=0; p<symbol->nparms; p++ )
                    {
                        node_t *param = global->children[1]->children[p];
                        symbol_t *psym = malloc ( sizeof(symbol_t) );
                        *psym = (symbol_t) {
                            .type = SYM_PARAMETER,
                            .name = param->data,
                            .node = NULL,
                            .seq = p,
                            .nparms = 0,
                            .locals = NULL
                        };
                        tlhash_insert (
                            symbol->locals, psym->name, strlen(psym->name), psym
                        );
                    }
                }
                add_global ( symbol );
                break;
            case DECLARATION:
                namelist = global->children[0];
                for ( uint64_t d=0; d<namelist->n_children; d++ )
                {
                    symbol = malloc ( sizeof(symbol_t) );
                    *symbol = (symbol_t) {
                        .type = SYM_GLOBAL_VAR,
                        .name = namelist->children[d]->data,
                        .node = NULL,
                        .seq = 0,
                        .nparms = 0,
                        .locals = NULL
                    };
                    add_global(symbol);
                }
                break;
        }
    }
}


static void
push_scope ( void )
{
    if ( scopes == NULL )
        scopes = malloc ( n_scopes * sizeof(tlhash_t *) );
    tlhash_t *new_scope = malloc ( sizeof(tlhash_t) );
    tlhash_init ( new_scope, 32 );
    scopes[scope_depth] = new_scope;

    scope_depth += 1;
    if ( scope_depth >= n_scopes )
    {
        n_scopes *= 2;
        scopes = realloc ( scopes, n_scopes*sizeof(tlhash_t **) );
    }

}


static void
add_local ( symbol_t *local )
{
    tlhash_insert (
        scopes[scope_depth-1],local->name,strlen(local->name),local
    );
}


static symbol_t *
lookup_local ( char *name )
{
    symbol_t *result = NULL;
    size_t depth = scope_depth;
    while ( result == NULL && depth > 0 )
    {
        depth -= 1;
        tlhash_lookup ( scopes[depth], name, strlen(name), (void **)&result );
    }
    return result;
}


static void
pop_scope ( void )
{
    scope_depth -= 1;
    tlhash_finalize ( scopes[scope_depth] );
    free ( scopes[scope_depth] );
    scopes[scope_depth] = NULL;
}


static void
add_string ( node_t *string )
{
    string_list[stringc] = string->data;
    string->data = malloc ( sizeof(size_t) );
    *((size_t *)string->data) = stringc;
    stringc++;
    if ( stringc >= n_string_list )
    {
        n_string_list *= 2;
        string_list = realloc ( string_list, n_string_list * sizeof(char *) );
    }
        
}


static void
bind_names ( symbol_t *function, node_t *root )
{
    if ( root == NULL )
        return;
    else switch ( root->type )
    {
        node_t *namelist;
        symbol_t *entry;

        case BLOCK:
            push_scope();
            for ( size_t c=0; c<root->n_children; c++ )
                bind_names ( function, root->children[c] );
            pop_scope();
            break;

        case DECLARATION:
            namelist = root->children[0];
            for ( uint64_t d=0; d<namelist->n_children; d++ )
            {
                node_t *varname = namelist->children[d];
                size_t local_num =
                    tlhash_size(function->locals) - function->nparms;
                symbol_t *symbol = malloc ( sizeof(symbol_t) );
                *symbol = (symbol_t) {
                    .type = SYM_LOCAL_VAR,
                    .name = varname->data,
                    .node = NULL,
                    .seq = local_num,
                    .nparms = 0,
                    .locals = NULL
                };
                tlhash_insert (
                    function->locals, &local_num, sizeof(size_t), symbol
                );
                add_local ( symbol );
            }
            break;

        case IDENTIFIER_DATA:
            entry = lookup_local ( root->data );
            if ( entry == NULL )
                tlhash_lookup (
                    function->locals, root->data,
                    strlen(root->data), (void**)&entry
                );
            if ( entry == NULL )
                tlhash_lookup (
                    global_names,root->data,strlen(root->data),(void**)&entry
                );
            if ( entry == NULL )
            {
                fprintf ( stderr, "Identifier '%s' does not exist in scope\n",
                    (char *)root->data
                );
                exit ( EXIT_FAILURE );
            }
            root->entry = entry;
            break;

        case STRING_DATA:
            add_string ( root );
            break;

        default:
            for ( size_t c=0; c<root->n_children; c++ )
                bind_names ( function, root->children[c] );
            break;
    }
}


void
destroy_symtab ( void )
{
    for ( size_t i=0; i<stringc; i++ )
        free ( string_list[i] );
    free ( string_list );

    size_t n_globals = tlhash_size ( global_names );
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );
    for ( size_t g=0; g<n_globals; g++ )
    {
        symbol_t *glob = global_list[g];
        if ( glob->locals != NULL )
        {
            size_t n_locals = tlhash_size ( glob->locals );
            symbol_t *locals[n_locals];
            tlhash_values ( glob->locals, (void **)&locals );
            for ( size_t l=0; l<n_locals; l++ )
                free ( locals[l] );
            tlhash_finalize ( glob->locals );
            free ( glob->locals );
        }
        free ( glob );
    }
    tlhash_finalize ( global_names );
    free ( global_names );
    free ( scopes );
}
