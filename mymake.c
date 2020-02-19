#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "digraph.h"
#include "mymake.h"
#include "helper_functions.h"
#include "digraph_extra.h"
#include "util.h"


struct mymake_t
{
	FILE * output;
	FILE * error;
	digraph_t * d;
};

digraph_t * get_digraph(mymake_t * m)
{
	return m->d;
}

// output and error are file handles that will be used by mymake to output
// additional information. For example, if mymake_add_variable returns
// false, there should have been an error message on the error file handle.
mymake_t * mymake_create(FILE * output, FILE * error)
{
	void destroy_cb (void * nodedata);
	mymake_t * mymake = (mymake_t *) malloc(sizeof(mymake_t));
	mymake->output = output;
	mymake->error = error;
	mymake->d = digraph_create(&destroy_cb);

	return mymake;
}

// Returns true if mymake_add_variable can be called.
// Note that even if this function returns false, mymake_add_variable
// needs to be implemented, but it is OK for it to always return false
// (as well as log an error on the error FILE (see above)).
bool mymake_supports_variables()
{
	return true;
}

bool mymake_add_variable(mymake_t * m, const char * varname, const char * val)
{
	return true;	
}

bool visit_cb(digraph_t * d, digraph_node_t * node, void * userdata)
{
	bool check_equality(char * s1, char * s2);
	if(check_equality((char *) digraph_node_get_data(node), (char *) userdata))
	{
		return true;
	}
	return false;
}

bool check_equality(char * s1, char * s2)
{
	if(strlen(s1) != strlen(s2)) return false;
	for(int i = 0; i < strlen(s1); i++)
	{
		if(s1[i] != s2[i]) return false;
	}
	return true;
}

/// Adds a new target. deps and recipe are NOT modified and the strings
/// they point to do not need to remain valid after this call returns.
/// Returns false if there was a problem (for example the target
/// already exists)
bool mymake_add_target(mymake_t * m, 
						const char * name, 
						const char ** deps, unsigned int depcount, 
						const char ** recipe, unsigned int recipecount)
{
	vararray * get_recipe(digraph_node_t * node);
	digraph_node_t * target_node;
	digraph_t * d = m->d;

	//process target
	digraph_node_t * node_found = digraph_find(d, &visit_cb, (char *) name);
	if(node_found != NULL) 
	{
		//this node already exists as a target
		if(digraph_node_outgoing_link_count(d, node_found) != 0) return false;

		//if node exists but doesnt have any outgoing links, it means it previously was only a dependency.
		//in this case we should proceed with processing dependencies, without readding the existing node
	}
	
	else target_node = digraph_node_create(d, (char *) name);	
	
	//process dependencies
	for(int i = 0; i < depcount; i++)
	{
		//check dependency does not already exist in our digraph
		digraph_node_t * find_dep = digraph_find(d, &visit_cb, (char *) deps[i]);

		if(find_dep == NULL)
		{

			digraph_node_t * dep_node = digraph_node_create(d, (char *) deps[i]);
			digraph_add_link(d, target_node, dep_node);	
		}
		else
		{
			digraph_add_link(d, target_node, find_dep);
		}
	}
	
	//process recipe
	for(int i = 0; i < recipecount; i++)
	{
		char * recipe_copy = (char *) malloc(strlen(recipe[i]));
		strcpy(recipe_copy, recipe[i]);

		vararray_add(get_recipe(target_node), recipe_copy, strlen(recipe[i]));
	}	

	return true;
}



// If target == 0, build the default target (the first target that
// was added).
// If target == 0 and there are no targets (and so no 'first' target), returns
// true.
//
// Returns false on error (for example a file with the name of the target
// doesn't exist and there is no recipe to build it).
// If the function returns false, an error message is written to the error
// file (passed in on mymake_create).
//
// If verbose is true, every target and dependency considered while building
// 'target' is written to
// the output file. If target is false, only the commands executed are written
// to output.
bool mymake_build(mymake_t * m, const char * target, bool verbose, bool dryrun)
{
	bool check_cycle(digraph_t * d, digraph_node_t * n);
	bool execute_dependencies(digraph_node_t * n, uint64_t parent_mod_time, FILE * output, FILE * error, bool dryrun, digraph_node_t * first_target);

	digraph_t * d = m->d;
	digraph_node_t * first_target;

	if(get_nodes(d)->length == 0)
	{
		fprintf(m->error, "No nodes in our digraph, do not know how to build target\n");
		return false;
	}

	if(target == 0) first_target = get_nodes(d)->array[0]; //no target specified, build default target	
	else first_target = digraph_find(d, &visit_cb, (char *) target); // target specified

	if(first_target == NULL) 
	{
		fprintf(m->error, "Could not find target %s\n", target);
		return false;
	}
	if(check_cycle(d, first_target)) 
	{
		fprintf(m->error, "Error: cycle in digraph , cannot build\n");
		return false;
	}
	
	//execute dependencies
	execute_dependencies(first_target, last_modification(target), m->output, m->error, dryrun, first_target);

	return true;
}


//using DFS to recurse down to base case, checking if our node was already seen. When we come back up from our
//depth to for loop back on other children, make sure to reset_nodes_seen before going back into depth
bool check_cycle(digraph_t * d, digraph_node_t * n)
{	
	bool output = false;
	bool seen = get_seen(n);

	if(seen == true) return true;
	else
	{
		set_seen(n, true);
		vararray * children = get_children(n);
		if(children->length != 0)
		{	
			for(int i = 0; i < children->length; i++)
			{
				output = check_cycle(d, children->array[i]);	
				reset_nodes_seen(d);
			}		
		}
		return output;
	}
}

// return value of children's node determines whether current node should execute.
// return value is calculated as true if any child nodes either executed or had parent_time < current_time
bool execute_dependencies(digraph_node_t * n, uint64_t parent_mod_time,
	FILE * output, FILE * error, bool dryrun, digraph_node_t * first_target)
{
	vararray * children = get_children(n);
	uint64_t current_mod_time = last_modification((char *) digraph_node_get_data(n));

	//check if parent's last modification is earlier than current node's last mod
	bool check_time = parent_mod_time < current_mod_time;
	bool check_executed = false;
	vararray * recipe = get_recipe(n);

	//base case
	if(children->length == 0)
	{
		//found dependency
		if(recipe->length == 0) 
		{
			//check if dependecy doesnt exist in our dir
			if(current_mod_time == 0)
			{
				fprintf(error, "No rule to make target %s, needed by %s \n",
					(char *) digraph_node_get_data(n), (char *) digraph_node_get_data(first_target));
				return false;
			}
			else
			{
				check_executed = false;
			}
		}
		
		//base case is not a dependency since recipe found, execute it
		else
		{
			//check if our dependency's recipe is not up to date or does not exist in our dir, throw an error
			if((current_mod_time == 0 || parent_mod_time > current_mod_time) && 
				digraph_node_get_data(n) != digraph_node_get_data(first_target))
			{
				fprintf(error, "No rule to make target %s, needed by %s \n",
					(char *) digraph_node_get_data(n), (char *) digraph_node_get_data(first_target));
				return false;
			}

			check_executed = execute_recipe((const char **) recipe->array, recipe->length, output, error, dryrun);
		}
	}

	//need to recurse down on children
	else
	{		
		for(int i = 0; i < children->length; i++)
		{
			//check the return value of every children, if any of them return true we should execute current node
			check_executed = (check_executed || 
				execute_dependencies(children->array[i], current_mod_time, output, error, dryrun, first_target));
		}
		if(check_executed) 
		{
			execute_recipe((const char **) recipe->array, recipe->length, stdout, error, dryrun);			
		}

	}

	return (check_executed || check_time);
}

// DOES NOT CLOSE THE FILES PASSED IN WITH mymake_create
void mymake_destroy(mymake_t * m)
{
	digraph_destroy(m->d);
	free(m);
}

void destroy_cb (void * nodedata)
{
	vararray_free(nodedata, sizeof(nodedata));
}

