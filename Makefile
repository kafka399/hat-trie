# List objects in order of DEPENDENCY. For example, if obj/main.o depends on
# obj/matrix.o, list obj/matrix.o first.
OBJECTS = obj/main.o
EXECUTABLE = bin/main

# make variables
OFLAGS   = -O2
CXX      = clang++
CXXFLAGS = -Wall -Wextra -c -I/opt/local/include -stdlib=libc++ $(OFLAGS)
LDFLAGS  = -lprofiler -stdlib=libc++

COMPILE.cpp = $(CXX) $(CXXFLAGS)

all: main

main: $(OBJECTS)
	$(CXX) $(OFLAGS) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

obj/%.o: src/%.cpp
	$(COMPILE.cpp) -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

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

obj/main.o: src/array-hash.h 
obj/main.o: src/hat-trie.h src/hat-trie-node.h
