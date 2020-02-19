#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Some helper functions for your mymake implementation
 */


/// Return the last modification time of the named file, in nanoseconds
/// since epoch.
///
/// Returns 0 if the file didn't exist.
/// Returns 1 if the file exists but there was an error obtaining the time.
/// (no permission, etc.).
uint64_t last_modification(const char * filename);

/// Executes the given recipe. Returns false if one of the commands failed,
/// true otherwise.
///
/// If dryrun == true, only output the commands to output, don't execute them,
/// and return true.
bool execute_recipe(const char ** recipe, unsigned int count, FILE * output,
        FILE * error, bool dryrun);


#endif