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
    /* TODO: Simplify the syntax tree structure 
    1. prune children: Delete nodes which can only ever have 1 child and no 
    meaningful data, and associate their child directly with their parent.

    2. resolve constant expressions: Compute the value of subtrees representing 
    arithmetic with constants, and replace them with their value.

    3. flatten: Delete internal nodes of list structures, leaving only a parent 
    node with a list type, and all list items as its children. Print list items 
    can be associated directly with the print statement.

    VARIABLE_LIST                           VARIABLE_LIST
      VARIABLE_LIST                           IDENTIFIER_DATA(i)
        VARIABLE_LIST                         IDENTIFIER_DATA(j)
          VARIABLE_LIST                       IDENTIFIER_DATA(k)
            VARIABLE_LIST                     IDENTIFIER_DATA(l)
              IDENTIFIER_DATA(i)  becomes     IDENTIFIER_DATA(m)
            IDENTIFIER_DATA(j)
          IDENTIFIER_DATA(k)
        IDENTIFIER_DATA(l)
      IDENTIFIER_DATA(m)

    Tip: implement these three steps as separate functions to complete one task
    at the time. e.g.:
        prune_children(root);
        redolve_constant_expressions(root);
        flatten(root);
        simplified = &root->children[0];
        node_finalize(root);
    */

}
