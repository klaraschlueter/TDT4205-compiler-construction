#ifndef IR_H
#define IR_H
#include <nodetypes.h>
#include <stdint.h>

/* This is the tree node structure */
typedef struct n {
    node_index_t type;      // Type of the node
    void *data;             // Pointer to associated data
    struct s *entry;        // Pointer to symtab entry (ignore for now)
    uint64_t n_children;    // Number of child nodes
    struct n **children;    // Array of n_children child nodes
} node_t;

/**Export the initializer function, it is needed by the parser
 * @param *nd node to initialize
 * @param type type of node (see nodetype.h)
 * @param *data associated data. Declared void to allow any type
 * @param n_children number of children
 * @param ... variable argument list of child nodes (node_t *)
 * */
void node_init (
    node_t *nd, node_index_t type, void *data, uint64_t n_children, ...
);
#endif
