#ifndef DIGRAPH_H
#define DIGRAPH_H

#include <stdbool.h>

struct digraph_node_t;

typedef struct digraph_node_t digraph_node_t;


struct digraph_t;
typedef struct digraph_t digraph_t;


typedef void (*digraph_destroy_cb_t) (void * nodedata);


// Create digraph. Stores the destroy_cb pointer, which is used
// by digraph_destroy and digraph_destroy_node. It is OK for 
// cb to be NULL.
digraph_t * digraph_create(digraph_destroy_cb_t cb);


/// Destroys all nodes and digraph; Calls the free function
/// for each destroyed node.
void digraph_destroy(digraph_t * graph);


digraph_node_t * digraph_node_create(digraph_t * d, void * userdata);

// Remove a node; Calls the destroy function on userdata (if not null)
void digraph_node_destroy(digraph_t * d, digraph_node_t * n);


// NOTE: it is *not* allowed to modify the graph/node structure
// from within this function
typedef bool (*digraph_visit_cb_t)(digraph_t * d,
                  digraph_node_t * node, void * userdata);

/// Enumerate all nodes, as long as the callback returns true.
/// Returns true if the callback always returned true,
/// false otherwise.
/// Userdata is passed to the callback
bool digraph_visit(digraph_t * g, digraph_visit_cb_t cb,
          void * userdata);

/// Visits all nodes; Returns the first node for which cb returns true,
/// returns false if there are no nodes or if cb never returned true.
digraph_node_t * digraph_find(digraph_t * g, digraph_visit_cb_t cb,
        void * userdata);

// Add a directed link between two nodes
void digraph_add_link(digraph_t * d, digraph_node_t * from, 
        digraph_node_t * to);

// Visit each outgoing link of the node
bool digraph_node_visit(digraph_t * d, digraph_node_t * n,
        digraph_visit_cb_t visit, void * userdata);

// Return number of outgoing links of the given node
unsigned int digraph_node_outgoing_link_count(const digraph_t * d, const digraph_node_t * n);

// Return number of incoming links of the given node
unsigned int digraph_node_incoming_link_count(const digraph_t * d, const
        digraph_node_t * n);

// Set data for given node. Returns the old value
void * digraph_node_set_data(digraph_t * d, digraph_node_t * n,
        void * userdata);

// Return data associated with node
// void * digraph_node_get_data(const digraph_t * d, const digraph_node_t * n);
void * digraph_node_get_data(const digraph_node_t * n);

        

#endif