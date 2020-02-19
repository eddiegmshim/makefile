#ifndef MYMAKE_H
#define MYMAKE_H

#include <stdio.h>
#include <stdbool.h>
#include "digraph.h"

struct mymake_t;
typedef struct mymake_t mymake_t;

// output and error are file handles that will be used by mymake to output
// additional information. For example, if mymake_add_variable returns
// false, there should have been an error message on the error file handle.
mymake_t * mymake_create(FILE * output, FILE * error);

// Returns true if mymake_add_variable can be called.
// Note that even if this function returns false, mymake_add_variable
// needs to be implemented, but it is OK for it to always return false
// (as well as log an error on the error FILE (see above)).
bool mymake_supports_variables();

bool mymake_add_variable(mymake_t * m, const char * varname, const char *
        val);

/// Adds a new target. deps and recipe are NOT modified and the strings
/// they point to do not need to remain valid after this call returns.
/// Returns false if there was a problem (for example the target
/// already exists)
bool mymake_add_target(mymake_t * m, const char * name, const char ** deps,
        unsigned int depcount, const char ** recipe, unsigned int recipecount);


// If target == 0, build the default target (the first target that
// was added).
// If there are no targets, returns true.
// Returns false on error (for example a file with the name of the target
// doesn't exist and there is no recipe to build it).
// If the function returns false, an error message is written to the error
// file (passed in on mymake_create).
//
// If verbose is true, every target and dependency considered is written to
// the output file. If target is false, only the commands executed are written
// to output.
bool mymake_build(mymake_t * m, const char * target, bool verbose, bool dryrun);

// DOES NOT CLOSE THE FILES PASSED IN WITH mymake_create
void mymake_destroy(mymake_t * m);

digraph_t * get_digraph(mymake_t * m);

#endif