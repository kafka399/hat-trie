# List objects in order of DEPENDENCY. For example, if obj/main.o depends on
# obj/matrix.o, list obj/matrix.o first.
OBJS = obj/main.o
EXE = bin/main
TESTOBJS = obj/array_hash_test.o 
TESTEXE = bin/test

# make variables
OFLAGS   = -O2
CXX      = /opt/llvm/bin/clang++
CXXFLAGS = -Wall -c 
LDFLAGS  = -lboost_unit_test_framework-mt -lprofile_rt -L/opt/llvm/lib

COMPILE.cpp = $(CXX) $(CXXFLAGS)

all: main

main: $(OBJS)
	$(CXX) $(OFLAGS) $(OBJS) -o $(EXE)

time: main
	time bin/main < inputs/kjv

test: $(TESTOBJS)
	$(CXX) --coverage -o $(TESTEXE) $(LDFLAGS) $(TESTOBJS)
	./$(TESTEXE)
	gcov -o obj test/array_hash_test.cpp > /dev/null
	gcov -o obj test/hat_set_test.cpp > /dev/null
	rm `ls *.gcov | grep -v "array_hash.h.gcov\|hat_trie.h.gcov"`

obj/%.o: src/%.cpp
	$(COMPILE.cpp) $(OFLAGS) -o $@ $<

obj/%.o: test/%.cpp
	$(COMPILE.cpp) --coverage -o $@ $<

clean:
	rm -f $(OBJS) $(EXE)
	rm -f $(TESTOBJS) $(TESTEXE)
	rm obj/*

depend:
	makedepend src/*.cpp 2>/dev/null
	echo "Now replace src/*.o with obj/*.o in Makefile"

# List dependencies here. Order doesn't matter.
# Ex:
#   obj/main.o: src/main.* src/matrix.*
#
# Assuming this directory structure:
#   Makefile
#   obj/
#   src/
# Run this command:
# 	makedepend src/*.cpp
# ... then change src/*.o in this Makefile to obj/*.o.
obj/array_hash_test.o: src/array_hash.h 
obj/hat_set_test.o: src/array_hash.h src/hat*
obj/main.o: src/array_hash.h src/main.cpp src/hat*
