#include <vslc.h>
#include <assert.h>
static void node_print ( node_t *root, int nesting );
static void simplify_tree ( node_t **simplified, node_t *root );
static void node_finalize ( node_t *discard );
static void prune_children ( node_t **root );
static void resolve_constant_expressions(node_t **root);
static void flatten(node_t **root);
static void flatten_inner(node_t **node);

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
    printf("\nroot address before: %p\n", root);
    simplify_tree ( &root, root );
    printf("\nroot address after: %p\n", root);
}


static void print_node(node_t node)
{
    printf("%s(%p){children=%ld}[", node_string[node.type], node.data, node.n_children);
    for (uint64_t i = 0; i < node.n_children; i++)
    {
        printf("%p", node.children[i]);
        if (i+1 < node.n_children)
            printf(", ");

    }
    printf("]\n");
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
        discard->type = 0;
        discard->n_children = 0;

        // printf("freeing data %p\n", discard->data );
        free ( discard->data );

        // printf("freeing children %p\n", discard->children );
        free ( discard->children );

        printf("freeing node %p\n", discard );
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
        // printf("Destroying %p\n", discard);
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
    prune_children (&root);
    resolve_constant_expressions(&root);
    flatten(&root);
    *simplified = root;         // make data of simplified (node_t*) be root
}


static void
prune_children( node_t** node )
{
    if (*node == NULL)
        return;
    if ((*node)->n_children == 1 && (*node)->data == NULL)
    {
        node_t* child = (*node)->children[0];
        printf("PRUNING: %s -> %s\n", node_string[(*node)->type], node_string[child->type]);
        node_finalize(*node);
        *node = child;
        prune_children(node);
    }
    else
    {
        for (uint64_t i = 0; i < (*node)->n_children; i++)
        {
            node_t** child = &(*node)->children[i];
            if (*child != NULL)
                prune_children(child);
        }
    }
}

static void resolve_constant_expressions(node_t **node)
{
    if (*node == NULL)
        return;
    for (uint64_t i = 0; i < (*node)->n_children; i++)
        resolve_constant_expressions(&(*node)->children[i]);

    if ((*node)->type == EXPRESSION)
    {
        if ((*node)->n_children == 1)
        {
            node_t* child_a = (*node)->children[0];
            if(child_a->type == NUMBER_DATA)
            {
                int64_t* value = (int64_t*)malloc(sizeof(int64_t));
                int64_t a = *(int64_t*)(child_a->data);
                switch (*(char*)(*node)->data)
                {
                case '-': *value =  -a; break;
                case '~': *value =  ~a; break;
                default:
                    printf("DEFAULT should not be reached!");
                    break;
                }
                node_finalize(child_a);
                node_finalize(*node);
                *node = (node_t*)malloc(sizeof(node_t));
                node_init(*node, NUMBER_DATA, value, 0);
            }
        }
        else if ((*node)->n_children == 2)
        {
            node_t* child_a = (*node)->children[0];
            node_t* child_b = (*node)->children[1];
            if(child_a->type == NUMBER_DATA && child_b->type == NUMBER_DATA)
            {
                int64_t* value = (int64_t*)malloc(sizeof(int64_t));
                int64_t a = *(int64_t*)(child_a->data);
                int64_t b = *(int64_t*)(child_b->data);
                switch (*(char*)(*node)->data)
                {
                case '+': *value =  a + b; break;
                case '-': *value =  a - b; break;
                case '*': *value =  a * b; break;
                case '/': *value =  a / b; break;
                case '|': *value =  a | b; break;
                case '^': *value =  a ^ b; break;
                case '&': *value =  a & b; break;
                default:
                    printf("DEFAULT should not be reached!");
                    break;
                }
                node_finalize(child_a);
                node_finalize(child_b);
                node_finalize(*node);
                *node = (node_t*)malloc(sizeof(node_t));
                node_init(*node, NUMBER_DATA, value, 0);
            }
        }
    }
}

/*
When we already are in a list node type, we don't want to keep checking node type
*/
static void flatten_inner(node_t **node)
{
    // ASSUMPTIONS: We have left fold lists, making every 0th (left) child a list
    //              or a list element, and every 1th(right) child a list element
    //            - No list post pruning will have a single child. The minimum is 2

    if ((*node)->n_children == 0 || (*node)->children[0]->n_children == 0)
        return;

    assert((*node)->n_children == 2 && "Node can only ever have two children at this point");

    // We want to start unraveling from the innermost part of the list, i.e. the start
    flatten_inner(&(*node)->children[0]); 

    uint64_t n_children = (*node)->n_children + (*node)->children[0]->n_children - 1;
    node_t* right = (*node)->children[1];
    printf("alloc'd: %p\n", (*node)->children[0]->children);
    node_t** left = (node_t**)realloc((*node)->children[0]->children, n_children * sizeof(node_t*));
    if (left == NULL)
        abort();
    
    printf("realloc'd: %p\n", left);

    left[(*node)->children[0]->n_children] = right; // Append the 'right' element
    //node_finalize((*node)->children[0]);            //free left side child, right we will keep

    // we're just about to overwrite this, we have grabbed the relevant pointers (right) out of it beforehand.
    free((*node)->children); 
    (*node)->children = left;
    (*node)->n_children = n_children;
}
static void flatten(node_t **node)
{
    // printf("%p\n", *node);
    if (*node == NULL)
        return;

    switch ((*node)->type)
    {
    case GLOBAL_LIST:
    case STATEMENT_LIST:
    case PRINT_LIST:
    case VARIABLE_LIST:
    case ARGUMENT_LIST:
    case EXPRESSION_LIST:
    case PARAMETER_LIST:
        flatten_inner(node);
        break;
    default:
        break;
    }
    for (uint64_t i = 0; i < (*node)->n_children; i++) 
        flatten(&(*node)->children[i]);
}