#define _POSIX_C_SOURCE 200809L

#include "util.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

uint64_t last_modification(const char * filename)
{
    struct stat statinfo;
    int ret = stat(filename, &statinfo);
    if (ret < 0)
    {
        // Check if file doesn't exist
        if (errno == ENOENT)
            return 0;

        // Some other error
        return 1;
    }

    return ((uint64_t) statinfo.st_mtim.tv_sec * 1000000000llu) + statinfo.st_mtim.tv_nsec;
}

bool execute_recipe(const char ** recipe, unsigned int count, FILE * output,
        FILE * error, bool dryrun)
{

    for (unsigned int i=0; i<count; ++i)
    {
        fprintf(output, "%s\n", recipe[i]);
        if (dryrun)
            continue;

        if (system(recipe[i]))
            return false;
        
    }
    return true;
}


