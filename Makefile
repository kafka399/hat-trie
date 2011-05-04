# List objects in order of DEPENDENCY. For example, if obj/main.o depends on
# obj/matrix.o, list obj/matrix.o first.
OBJECTS = obj/array-hash.o obj/hat-trie-node.o obj/hat-trie.o obj/main.o
EXECUTABLE = bin/main

# make variables
OFLAGS   = -O2
CXX      = clang++
CXXFLAGS = -Wall -c $(OFLAGS) 
LDFLAGS  = -lprofiler 

COMPILE.cpp = $(CXX) $(CXXFLAGS)

all: main

main: $(OBJECTS)
	$(CXX) $(OFLAGS) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

test: all
	./$(EXECUTABLE)

obj/%.o: src/%.cpp
	$(COMPILE.cpp) -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

doc:
	doxygen

gen: src/generator.cpp
	$(CXX) $(OFLAGS) src/generator.cpp -o bin/generator

# List dependencies here. Order doesn't matter. makedepend src/*.cpp
# works perfectly for this section.
# Ex:
#   obj/main.o: src/main.* src/matrix.*
obj/hat-trie-node.o: obj/array-hash.o src/hat-trie-node.cpp
