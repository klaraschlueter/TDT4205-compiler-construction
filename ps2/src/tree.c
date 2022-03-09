//#include <vslc.h>
#include <ir.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct stem_t *stem;
struct stem_t { const char *str; stem next; };

static void node_print ( node_t *root, int nesting );
static void node_finalize ( node_t *discard );
static void destroy_subtree ( node_t *discard );
static void tree_print ( node_t *root, stem head );

node_t *root;

/* External interface */
void
destroy_syntax_tree ( void )
{
    destroy_subtree ( root );
}


void
print_syntax_tree ( void )
{
    if (root == NULL) {
        printf ("Printing empty tree.");
        return;
    }
    tree_print ( root, 0 );
}

static void
tree_print(node_t* root, stem head) 
{
    if (root == NULL) {
        printf("tree_print got passed a node that is NULL!");
    }
    static const char *sdown = "  │", *slast = "  └", *snone = "   ";
    struct stem_t col = {0, 0}, *tail;

    for (tail = head; tail; tail = tail->next) {
        if (!tail->next) {
            if (!strcmp(sdown, tail->str))
                printf("  ├");
            else 
                printf("%s", tail->str);
            break;
        }
        printf("%s", tail->str);
    }
    
    printf("──%s", node_string[root->type] );
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


// Are we getting pointers to the children, or the actual children? (Pretending for now that we get pointers)
void
node_init (node_t *nd, node_index_t type, void *data, uint64_t n_children, ...)
{
    nd->type = type;
    nd->data = data;
    nd->n_children = n_children;
    
    // allocate space for the pointers
    // set nd->children to a pointer to the allocated space
    nd->children = malloc(n_children);
    
    
    // get pointers to children
    va_list children;
    va_start(children, n_children);
    
    // and save the pointers in the allocated space
    for ( int64_t i=0; i < n_children; i++ ) {
        nd->children[i] = va_arg(children, node_t*);
    }
    va_end(children);
    // TODO: if possible, check amount of children against n_children
}


static void
node_print ( node_t *r, int nesting )
{
    if ( r != NULL )
    {
        printf ( "%*c%s", nesting, ' ', node_string[r->type] );
        if ( r->type == IDENTIFIER_DATA ||
             r->type == STRING_DATA ||
             r->type == EXPRESSION ) 
            printf ( "(%s)", (char *) r->data );
        else if ( r->type == NUMBER_DATA )
            printf ( "(%ld)", *((int64_t *)r->data) );
        putchar ( '\n' );
        for ( int64_t i=0; i < r->n_children; i++ )
            node_print ( r->children[i], nesting+1 );
    }
    else
        printf ( "%*c%p\n", nesting, ' ', r );
}


static void
node_finalize ( node_t *discard )
{
    free(discard->children);
    free(discard->data); // tree construction in parser allocates space for data
    free(discard);
}


static void
destroy_subtree ( node_t *discard )
{
    /* Remove all nodes in the subtree rooted at a node */
    if (discard->n_children == 0) {
        node_finalize(discard);
    } else {
        for ( int64_t i=0; i < discard->n_children; i++ ) {
            destroy_subtree(discard->children[i]);
        }
    }
    
    // case we at a leaf?
    //    call node_finalize
    // case we at an inner node
    //    loop over children
    //         call destroy_subtree for child
}
