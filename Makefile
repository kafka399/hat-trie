# List objects in order of DEPENDENCY. For example, if obj/main.o depends on
# obj/matrix.o, list obj/matrix.o first.
OBJECTS = 
EXE = bin/main

TESTOBJS = obj/array_hash_test.o
TESTEXE = bin/test

# make variables
OFLAGS   = -O0
CXX      = gcc-4.2 --coverage
CXXFLAGS = -Wall -c $(OFLAGS) 
LDFLAGS  = -lboost_unit_test_framework-mt -lstdc++

COMPILE.cpp = $(CXX) $(CXXFLAGS)

all: test

main: $(OBJECTS)
	$(CXX) $(OFLAGS) $(OBJECTS) -o $(EXE) $(LDFLAGS)

test: $(TESTOBJS)
	$(CXX) $(OFLAGS) $(TESTOBJS) -o $(TESTEXE) $(LDFLAGS)

cover: $(TESTOBJS)
	$(CXX) --coverage -o $(TESTEXE) $(LDFLAGS) $(TESTOBJS)
	./$(TESTEXE)
	gcov -o obj test/array_hash_test.cpp > /dev/null
	rm `ls *.gcov | grep -v array_hash.h.gcov`

obj/%.o: src/%.cpp
	$(COMPILE.cpp) -o $@ $<

obj/%.o: test/%.cpp
	$(COMPILE.cpp) -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXE)
	rm -f $(TESTOBJS) $(TESTEXE)

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
obj/array_hash_test.o: src/array_hash.h test/array_hash_test.cpp
