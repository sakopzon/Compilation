# Makefile to build Project part1 + part2

all: part2

part2: part2.hpp part2.tab.cpp part2.tab.hpp part1.c part1.h
	g++ part2.tab.cpp part1.c -o part2

part1.c part1.h: part1.lex
	flex part1.lex
	
part2.tab.cpp part2.tab.hpp: part2.ypp part2.hpp
	bison -d part2.ypp

# Utility targets
.PHONY: clean test
clean:
	rm -f part1 part1.c part1.h part2 part2.ypp part2.hpp part2.tab.cpp part2.tab.hpp

test: part1
	./run_tests.sh
