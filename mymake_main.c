#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "makefile_parser.h"
#include "mymake.h"
#include "digraph.h"
#include "digraph_extra.h"

bool mymake_add_target_helper(mymake_t * m,
        const char ** targets, unsigned int tcount,
        const char ** dependencies, unsigned int dcount,
        const char ** recipe, unsigned int rcount)
{
	char * target;
	bool return_bool;

	//loop through each target and add into digraph
	for(int i = 0; i < tcount; i++)
	{
		target = (char *) targets[i];	
		return_bool = mymake_add_target(m, target, dependencies, dcount, recipe, rcount);
	}
	return return_bool;
}

bool cb_target_adder(void * userdata,
        const char ** target, unsigned int tcount,
        const char ** dependencies, unsigned int dcount,
        const char ** recipe, unsigned int rcount)
{
	bool out = mymake_add_target_helper((mymake_t *) userdata,
		target, tcount,
        dependencies, dcount,
        recipe, rcount);

	return out;
}

int main(int argc, char ** argv)
{
	int opt;
	bool verbose_mode = false;
	bool dryrun_mode = false;
	char * filename;
	int arg_counter = 1;

	while((opt = getopt(argc, argv, "hvnf:")) != -1)
	{
		switch(opt)
		{
			//outputs: mymake [-f filename] [-v] [-n] target...
			case 'h':
				printf("mymake ");
				for(int i = 0; i < argc; i++)
				{
					printf("%s ", argv[i]);
				}
				printf("\n");		
				arg_counter++;
				break;

			//enables verbose mode
			case 'v':
				verbose_mode = true;
				arg_counter++;
				break;

			//dryrun mode
			case 'n':
				dryrun_mode = true;
				arg_counter++;
				break;

			//takes one argument, which is name of the mymake file to read. If -f is passed as an 
			//argument on the command line, the file to read should default to Makefile.mymake
			case 'f':
				filename = optarg;
				if(filename[0] == '-')
				{
					arg_counter++;
					if(filename[1] == 'v')
					{
						verbose_mode = true;	
					}
					else if(filename[1] == 'n')
					{
						dryrun_mode = true;
					}
					else if(filename[1] == 'h')
					{
						printf("mymake ");
						for(int i = 0; i < argc; i++)
						{
							printf("%s ", argv[i]);
						}
						printf("\n");		
						break;
					}

					filename = "Makefile.mymake";
				}
				arg_counter = arg_counter +2;
				break;	
		}
	}

	FILE * input_file = fopen(filename, "r");
	FILE * o = fopen("open_test.txt", "w");
	FILE * e = fopen("error_test", "w");
	mymake_t * m = mymake_create(o,e);

	struct mfp_cb_t mycb;
	mycb.rule_cb = &cb_target_adder;
	mycb.error = e;

	mfp_parse(input_file, &mycb, m);

	// loop through each target given by command line and build
	for(int i = arg_counter; i < argc; i++)
	{	
		mymake_build(m, argv[i], verbose_mode, dryrun_mode);
	}

	return EXIT_SUCCESS;
}


