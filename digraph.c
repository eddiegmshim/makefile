#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "digraph.h"
#include "digraph_extra.h"
#include "helper_functions.h"

// struct digraph_node_t;
// typedef struct digraph_node_t digraph_node_t;


struct digraph_node_t
{
	void * data;
	vararray * children;
	vararray * parents;
	vararray * recipe;
	bool seen;
};

// struct digraph_t;
// typedef struct digraph_t digraph_t;

struct digraph_t
{
	vararray * nodes;
	digraph_destroy_cb_t cb;
};

// typedef void (*digraph_destroy_cb_t) (void * nodedata);


// Create digraph. Stores the destroy_cb pointer, which is used
// by digraph_destroy and digraph_destroy_node. It is OK for 
// cb to be NULL.
digraph_t * digraph_create(digraph_destroy_cb_t cb)
{
	vararray * init_vararray();
	digraph_t * d = (digraph_t *) malloc(sizeof(digraph_t));
	d -> nodes = init_vararray();
	d -> cb = cb;
	return d;
}

// / Destroys all nodes and digraph; Calls the free function
// / for each destroyed node.
void digraph_destroy(digraph_t * graph)
{		
	void free_node(digraph_node_t * node);
	for(int i = 0; i < graph->nodes->length; i++)
	{
		digraph_node_t * node = graph->nodes->array[i];	

		// call destroy cb
		if(graph->cb != NULL) (graph->cb)(node->data);

		//free node
		free_node(node);
	}
	free(graph->nodes->array);
	free(graph->nodes);
	free(graph);	
}

void free_node(digraph_node_t * node)
{	
	free(node->data);
	free(node->children->array);
	free(node->children);
	free(node->parents->array);
	free(node->parents);
	free(node->recipe->array);
	free(node->recipe);
	free(node);
}

digraph_node_t * digraph_node_create(digraph_t * d, void * userdata)
{
	digraph_node_t * node = (digraph_node_t *) malloc(sizeof(digraph_node_t));
	node -> data = (void *) malloc(strlen(userdata));
	memcpy(node->data, userdata, strlen(userdata));

	node -> children = init_vararray();
	node -> parents = init_vararray();
	node -> recipe = init_vararray();
	node -> seen = false;

	vararray_add(d->nodes, node, sizeof(digraph_node_t *));
	return node;
}

// Remove a node; Calls the destroy function on userdata (if not null)
void digraph_node_destroy(digraph_t * d, digraph_node_t * n)
{
	void remove_dependencies(vararray * children, vararray * parents, digraph_node_t * target);
	vararray * nodes = d -> nodes;

	//scroll through our nodes to see if we have our target
	for(int i = 0; i < nodes->length; i++)
	{		
		if(n == ((digraph_node_t *) nodes->array[i]))
		{
			//call destroy function on userdata (if not null)
			if(d->cb !=NULL) d->cb(n->data);

			//first remove all dependencies
			remove_dependencies(n->children, n->parents, n);

			//second remove node 
			vararray_remove(nodes, n, i);				
			free_node(n);
			break;
		}
	}		
}

void remove_dependencies(vararray * children, vararray * parents, digraph_node_t * target)
{
	//remove target's children's parent dependency
	for(int i = 0; i < children -> length; i++)
	{
		digraph_node_t * child = children->array[i];
		for(int i = 0; i < child->parents->length; i++)
		{
			if(child->parents->array[i] == target) vararray_remove(child->parents, target, i);
		}		
	}
	//remove target's parent's child dependency
	for(int i = 0; i < parents -> length; i++)
	{
		digraph_node_t * parent = parents->array[i];
		for(int i = 0; i < parent->children->length; i++)
		{
			if(parent->children->array[i] == target) vararray_remove(parent->children, target, i);
		}
	}
}

void print_digraph(digraph_t * d)
{
	vararray * nodes = d -> nodes;
	for(int i = 0; i < nodes->length; i++)
	{
		digraph_node_t * node = nodes->array[i];
		printf("Node: %d, data: %s\n", i, (char *) (node->data));
		printf("Node: %d, parentLength: %d\n", i, node->parents->length);
		printf("Node: %d, childrenLength: %d\n", i, node->children->length);
	}
}


/// Enumerate all nodes, as long as the callback returns true.
/// Returns true if the callback always returned true,
/// false otherwise.
/// Userdata is passed to the callback
bool digraph_visit(digraph_t * g, digraph_visit_cb_t cb,
          void * userdata)
{
	bool return_bool = true;

	for(int i = 0; i < g->nodes->length; i++)
	{
		digraph_node_t * node = g->nodes->array[i];
		return_bool = cb(g, node, node->data);
		if(return_bool == false) return false;
	}

	return return_bool;
}

/// Visits all nodes; Returns the first node for which cb returns true,
/// returns NULL if there are no nodes or if cb never returned true.
digraph_node_t * digraph_find(digraph_t * g, digraph_visit_cb_t cb, void * userdata)
{
	bool return_bool;

	for(int i = 0; i < g->nodes->length; i++)
	{
		digraph_node_t * node = g->nodes->array[i];
		return_bool = cb(g, node, userdata);
		if(return_bool == true) 
		{
			return node;
		}
	}

	return NULL;
}

// Add a directed link between two nodes
void digraph_add_link(digraph_t * d, digraph_node_t * from, 
        digraph_node_t * to)
{
	//check if link already exists
	for(int i = 0; i < from->children->length; i++)
	{
		if(from->children->array[i] == to) return;
	}

	vararray_add(from->children, to, sizeof(to));
	vararray_add(to->parents, from, sizeof(from));
}

// Visit each outgoing link of the node
bool digraph_node_visit(digraph_t * d, digraph_node_t * n,
        digraph_visit_cb_t visit, void * userdata)
{
	bool return_bool = true;

	vararray * children = n->children;
	for(int i = 0; i < children->length; i++)
	{
		digraph_node_t * child = children->array[i];
		return_bool = visit(d, child, child->data);
	}

	return return_bool;
}

// Return number of outgoing links of the given node
unsigned int digraph_node_outgoing_link_count(const digraph_t * d, const digraph_node_t * n)
{
	return n->children->length;
}

// Return number of incoming links of the given node
unsigned int digraph_node_incoming_link_count(const digraph_t * d, const
        digraph_node_t * n)
{
	return n->parents->length;
}

// Set data for given node. Returns the old value
void * digraph_node_set_data(digraph_t * d, digraph_node_t * n,
        void * userdata)
{
	void * old_data = n->data;
	n->data = userdata;
	return old_data;
}

// Return data associated with node
void * digraph_node_get_data(const digraph_node_t * n)
{
	return n->data;
}       

vararray * get_recipe(digraph_node_t * node)
{
	return node->recipe;
}

vararray * get_nodes(digraph_t * d)
{
	return d->nodes;
}

vararray * get_children(digraph_node_t * n)
{
	return n->children;
}

bool get_seen(digraph_node_t * n)
{
	return n->seen;
}

void set_seen(digraph_node_t * n, bool input)
{
	n->seen = input;
}

void reset_nodes_seen(digraph_t * d)
{
	for(int i = 0; i < d->nodes->length; i++)
	{
		digraph_node_t * n = d->nodes->array[i];
		n->seen = false;
	}
} 
