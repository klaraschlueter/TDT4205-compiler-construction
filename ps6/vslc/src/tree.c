#include <vslc.h>

static void node_print ( node_t *root, int nesting );
static void simplify_tree ( node_t **simplified, node_t *root );
static void node_finalize ( node_t *discard );

typedef struct stem_t *stem;
struct stem_t { const char *str; stem next; };
static void
tree_print(node_t* root, stem head);


static void destroy_subtree ( node_t *discard );


/* External interface */
void
destroy_syntax_tree ( void )
{
    destroy_subtree ( root );
}


void
simplify_syntax_tree ( void )
{
    simplify_tree ( &root, root );
}


extern bool new_print_style;
void
print_syntax_tree ( void )
{
    if (new_print_style)
        tree_print ( root, 0 );
    // Old tree printing
    else
        node_print ( root, 0 );
}


void
node_init (node_t *nd, node_index_t type, void *data, uint64_t n_children, ...)
{
    va_list child_list;
    *nd = (node_t) {
        .type = type,
        .data = data,
        .entry = NULL,
        .n_children = n_children,
        .children = (node_t **) malloc ( n_children * sizeof(node_t *) )
    };
    va_start ( child_list, n_children );
    for ( uint64_t i=0; i<n_children; i++ )
        nd->children[i] = va_arg ( child_list, node_t * );
    va_end ( child_list );
}


static void
tree_print(node_t* root, stem head) 
{
    static const char *sdown = " │", *slast = " └", *snone = "  ";
    struct stem_t col = {0, 0}, *tail;

    // Print stems of branches coming further down
    for (tail = head; tail; tail = tail->next) {
        if (!tail->next) {
            if (!strcmp(sdown, tail->str))
                printf(" ├");
            else 
                printf("%s", tail->str);
            break;
        }
        printf("%s", tail->str);
    }
    
    if (root == NULL) {
        // Secure against null pointers sent as root
        printf("─(nil)\n");
        return;
    }
    printf("─%s", node_string[root->type]);
    if ( root->type == IDENTIFIER_DATA ||
         root->type == STRING_DATA ||
         root->type == EXPRESSION ) 
        printf("(%s)", (char *) root->data);
    else if (root->type == NUMBER_DATA)
        printf("(%ld)", *((int64_t *)root->data));
    putchar('\n');
 
    if (!root->n_children) return;
 
    if (tail && tail->str == slast)
        tail->str = snone;
 
    if (!tail)  tail = head = &col;
    else        tail->next = &col;

    for ( int64_t i=0; i < root->n_children; i++ ) {
        col.str = root->n_children - i - 1 ? sdown : slast;
        tree_print(root->children[i], head);
    }
    tail->next = 0;
}

/* Internal choices */
static void
node_print ( node_t *root, int nesting )
{
    if ( root != NULL )
    {
        printf ( "%*c%s", nesting, ' ', node_string[root->type] );
        if ( root->type == IDENTIFIER_DATA ||
             root->type == STRING_DATA ||
             root->type == EXPRESSION ) 
            printf ( "(%s)", (char *) root->data );
        else if ( root->type == NUMBER_DATA )
            printf ( "(%ld)", *((int64_t *)root->data) );
        putchar ( '\n' );
        for ( int64_t i=0; i<root->n_children; i++ )
            node_print ( root->children[i], nesting+1 );
    }
    else
        printf ( "%*c%p\n", nesting, ' ', root );
}


static void
node_finalize ( node_t *discard )
{
    if ( discard != NULL )
    {
        free ( discard->data );
        free ( discard->children );
        free ( discard );
    }
}


static void
destroy_subtree ( node_t *discard )
{
    if ( discard != NULL )
    {
        for ( uint64_t i=0; i<discard->n_children; i++ )
            destroy_subtree ( discard->children[i] );
        node_finalize ( discard );
    }
}


static void
simplify_tree ( node_t **simplified, node_t *root )
{
    if ( root == NULL )
        return;

    /* Simplify subtrees before examining this node */
    for ( uint64_t i=0; i<root->n_children; i++ )
        simplify_tree ( &root->children[i], root->children[i] );

    node_t *discard, *result = root;
    switch ( root->type )
    {
        /* Structures of purely syntactic function */
        case PARAMETER_LIST: case ARGUMENT_LIST:
        case STATEMENT: case PRINT_ITEM: case GLOBAL:
            result = root->children[0];
            node_finalize ( root );
            break;
        case PRINT_STATEMENT:
            result = root->children[0];
            result->type = PRINT_STATEMENT;
            node_finalize(root);
        /* Flatten lists:
         * Take left child, append right child, substitute left for root.
         */
        case STATEMENT_LIST: case DECLARATION_LIST: case GLOBAL_LIST:
        case PRINT_LIST: case EXPRESSION_LIST: case VARIABLE_LIST:
            if ( root->n_children >= 2 )
            {
                result = root->children[0];
                result->n_children += 1;
                result->children = realloc (
                    result->children, result->n_children * sizeof(node_t *)
                );
                result->children[result->n_children-1] = root->children[1];
                node_finalize ( root );
            }
            break;
        case EXPRESSION:
            switch ( root->n_children )
            {
                case 1:
                    if ( root->children[0]->type == NUMBER_DATA )
                    {
                        result = root->children[0];
                        if ( root->data != NULL )
                            *((int64_t *)result->data) *= -1;
                        node_finalize (root);
                    }
                    else if ( root->data == NULL )
                    {
                        result = root->children[0];
                        node_finalize (root);
                    }
                    break;
                case 2:
                    if ( root->children[0]->type == NUMBER_DATA &&
                         root->children[1]->type == NUMBER_DATA
                    ) {
                        result = root->children[0];
                        int64_t
                            *x = result->data,
                            *y = root->children[1]->data;
                        switch ( *((char *)root->data) )
                        {
                            case '+': *x += *y; break;
                            case '-': *x -= *y; break;
                            case '*': *x *= *y; break;
                            case '/': *x /= *y; break;
                        }
                        node_finalize ( root->children[1] );
                        node_finalize ( root );
                    }
                    break;
            }
    }
    *simplified = result;
}
