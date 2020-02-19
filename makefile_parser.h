#ifndef MAKEFILE_PARSER_H
#define MAKEFILE_PARSER_H

#include <stdbool.h>
#include <stdio.h>

// This file needs to #define MFP_SUPPORT_VARIABLES if the parser
// supports variables
#include "makefile_parser_config.h"


/**
 * MyMake syntax:
 *
 * NOTE:
 *    - to keeps parsing simple, we do not need to support strings (")
 *      which means we cannot handle filenames which have spaces in them.
 *
 * === Comments ====
 *    Whenever a # is found the # and any characters
 *    until the rest of the line should be ignored.
 *
 * === Variables (if MFP_SUPPORT_VARIABLES is defined) ====
 *
 * Syntax:
 *    IDENTIFIER=VALUE
 *
 *    IDENTIFIER starts with [a-z,A-Z,_] and is followed by zero or more
 *               of [a-z,A-Z,0-9,_]. Spaces are not allowed in an identifier.
 *
 *    VALUE is everything following =, up to but not including the newline
 *    (\n). If there is are spaces following the =, these should be included
 *    in the value of the variable.
 *
 * ==== Rules & recipes ====
 *
 *   target and dependencies cannot contain spaces. Spaces are used to
 *   separate different targets/dependencies. Targets and dependencies
 *   cannot contain '='.
 *   The valid characters for target and dependency are:
 *      [a-z,A-Z,0-9,_,.,-] as well as / (directory separator)
 *
 *   !!Any other character in a target or dependency should result in an error!!
 *
 *   Recipe: If the first line following a rule starts with a <TAB> character
 *   (\t) then the rule has an associated recipe block.
 *
 *   Note that a blank line (containing only whitespace) does not end
 *   a recipe block. Only a non-blank line with a non-blank character in the
 *   first column does so. In particular, the following should create a single
 *   recipe block:
 *
 *   test:
 *         cmd1
 *         cmd2
 *
 *         cmd3
 *
 *  Rules start with a target in the first column of the line, followed
 *  by a colon, and one or more optional dependencies.
 *
 *  Any lines (ignoring blank lines, see above) starting with a tab character
 *  (\t) in the first column list the commands that need to be executed
 *  to update the target. These commands are optional.
 *
 *  It is OK to have multiple rules for a target. Your make program itself
 *  should not allow multiple recipes for a target however, but this is not
 *  something the parser should check or enforce.
 *
 *  If your makefile_parser_config.h defines MFP_SUPPORT_MULTITARGET
 *  then there can be multiple targets in a single recipe.
 *  Example:
 *
 *  a b c: dep1 dep2
 *
 *  !! THERE SHOULD BE NO ARTIFICIAL LIMITATIONS ON THE NUMBER OF           !!
 *  !! TARGETS/RULES/RECIPE LENGTH/LENGTH OF VARIABLE NAMES                 !!
 *  !! LENGTH OF A LINE/...                                                 !!
 *
 *  !! Your code is not allowed to use static local variables or            !!
 *  !! global variables!                                                    !!
 */


/// Pointer to a function called when a variable definition is found in the file.
/// IMPORTANT: varname and value only remain valid for the duration of the call
/// to the function.
///
///   line is the line number of the file where the variable was found.
///   (first line is line == 1)
///

typedef bool (*mfp_variable_cb_t) (void * userdata,
        unsigned int line, 
        const char * varname, 
        const char * value);

/// Pointer to a function which will be called when a target is found.
/// Targets might have optional dependencies as well as optional recipe
/// instructions.
///
/// Line is the line number of the target
///
///   target, dependencies and recipe are arrays of const char *,
///   where the number of elements in the array is indicated by
///   tcount, dcount and rcount respectively.
///
///   If MFP_SUPPORT_MULTITARGET is not defined, tcount will always be 1.
///
///   Arguments passed to the callback only remain valid for the duration
///   of the call.
///
///   If the callback returns false, parsing will stop.
///
typedef bool (*mfp_rule_cb_t) (void * userdata,
        const char ** target, unsigned int tcount,
        const char ** dependencies, unsigned int dcount,
        const char ** recipe, unsigned int rcount);

struct mfp_cb_t
{
#ifdef MFP_SUPPORT_VARIABLES
    mfp_variable_cb_t  variable_cb;
#endif
    mfp_rule_cb_t rule_cb;
    FILE * error;
};

typedef struct mfp_cb_t mfp_cb_t;

/// Returns true if parsing succeeded, otherwise returns false.
/// If any of the callbacks return false, parsing stops immediately
/// and the function returns false.
///
/// Any errors should be written to cb->error.
/// This function should NOT fclose cb->error
bool mfp_parse(FILE * f, const mfp_cb_t * cb, void * extradata);


#endif