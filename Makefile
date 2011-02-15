# List objects in order of DEPENDENCY. For example, if obj/main.o depends on
# obj/matrix.o, list obj/matrix.o first.
OBJECTS = obj/main.o
EXECUTABLE = bin/main

# make variables
OFLAGS   = -O2
CXX      = g++
CXXFLAGS = -Wall -c $(OFLAGS)
LDFLAGS  =

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

# List dependencies here. Order doesn't matter. makedepend src/*.cpp
# works perfectly for this section.
# Ex:
#   obj/main.o: src/main.* src/matrix.*
obj/main.o: src/main.cpp
obj/main.o: src/hat-trie*
obj/main.o: src/array-hash.h
