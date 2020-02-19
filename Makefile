all: mymake_main.c makefile_parser.c helper_functions.c mymake.c makefile_parser.h helper_functions.h mymake.h util.c util.h digraph.h digraph.c digraph_extra.h makefile_parser.c makefile_parser_driver.c helper_functions.c
	gcc -pedantic -Wall -std=c11 -o mymake mymake_main.c makefile_parser.c helper_functions.c mymake.c makefile_parser.h helper_functions.h mymake.h util.c util.h digraph.h digraph.c digraph_extra.h
	gcc -pedantic -Wall -std=c11 -o makefile_parser makefile_parser.c makefile_parser_driver.c helper_functions.c
	./makefile_parser Makefile.mymake
	./mymake -f Makefile.mymake t1

mymake: mymake_main.c makefile_parser.c helper_functions.c mymake.c makefile_parser.h helper_functions.h mymake.h util.c util.h digraph.h digraph.c digraph_extra.h
	gcc -pedantic -Wall -std=c11 -o mymake mymake_main.c makefile_parser.c helper_functions.c mymake.c makefile_parser.h helper_functions.h mymake.h util.c util.h digraph.h digraph.c digraph_extra.h
	./mymake -f Makefile.mymake t1

makefile_parser_driver: makefile_parser.c makefile_parser_driver.c helper_functions.c
	gcc -pedantic -Wall -std=c11 -o makefile_parser makefile_parser.c makefile_parser_driver.c helper_functions.c
	./makefile_parser Makefile.mymake

clean:
	rm mymake