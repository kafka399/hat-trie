# List objects in order of DEPENDENCY. For example, if obj/main.o depends on
# obj/matrix.o, list obj/matrix.o first.
OBJECTS = obj/array-hash.o obj/hat-trie-node.o obj/hat-trie.o obj/main.o
EXECUTABLE = bin/main

# make variables
OFLAGS   = -O2
CXX      = clang++
CXXFLAGS = -Wall -c -I/opt/local/include -stdlib=libc++ $(OFLAGS)
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
	echo "Now change src/*.o to obj/*.o in Makefile"
	rm Makefile.bak

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

src/MurmurHash3.o: src/MurmurHash3.h /usr/include/stdint.h
src/array-hash.o: src/array-hash.h /usr/include/stdint.h
src/hat-trie-node.o: src/hat-trie-node.h src/array-hash.h
src/hat-trie-node.o: /usr/include/stdint.h
src/hat-trie.o: src/hat-trie.h src/array-hash.h /usr/include/stdint.h
src/hat-trie.o: src/hat-trie-node.h
src/main.o: /usr/include/time.h /usr/include/_types.h
src/main.o: /usr/include/sys/_types.h /usr/include/sys/cdefs.h
src/main.o: /usr/include/machine/_types.h /usr/include/i386/_types.h
src/main.o: /usr/include/_structs.h /usr/include/sys/_structs.h
src/main.o: /usr/include/unistd.h /usr/include/sys/unistd.h
src/main.o: /usr/include/sys/select.h /usr/include/sys/appleapiopts.h
src/main.o: /usr/include/sys/_select.h src/array-hash.h /usr/include/stdint.h
src/main.o: src/hat-trie.h src/hat-trie-node.h
