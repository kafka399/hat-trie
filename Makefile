# List objects in order of DEPENDENCY. For example, if obj/main.o depends on
# obj/matrix.o, list obj/matrix.o first.
TESTOBJECTS = obj/array-hash-test.o obj/trie-test.o obj/Test.o 
TESTEXE = bin/test
EXE = bin/main

# make variables
OFLAGS   = -O0
CXX      = gcc-4.2
CXXFLAGS = -Wall -c -Icute/ -Iboost/ $(OFLAGS)
LDFLAGS  = -lstdc++

COMPILE.cpp = $(CXX) $(CXXFLAGS)

all: cover

main: $(OBJECTS)
	$(CXX) $(OFLAGS) $(OBJECTS) -o $(EXE) $(LDFLAGS)

cover: $(TESTOBJECTS)
#	rm -f *.gcov obj/*.gcno obj/*.gcda
	$(CXX) --coverage -o $(TESTEXE) $(LDFLAGS) $(TESTOBJECTS)
	./bin/test > /dev/null
	gcov -o obj test/array-hash-test.cpp > /dev/null
	rm `ls *.gcov | grep -v array_hash.h.gcov`

obj/%.o: test/%.cpp
	$(COMPILE.cpp) --coverage -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(TESTEXE) $(TESTOBJECTS) *.gcov

depend:
	makedepend src/*.cpp 2>/dev/null
	rm Makefile.bak
	echo "========================================="
	echo "Now change src/*.o to obj/*.o in Makefile"

# List dependencies here. Order doesn't matter. makedepend src/*.cpp
# works perfectly for this section.
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
# DO NOT DELETE

obj/Test.o: test/array-hash-test.h src/array_hash.h 
obj/Test.o: test/trie-test.h
obj/array-hash-test.o: test/array-hash-test.h src/array_hash.h
obj/trie-test.o: test/trie-test.h
