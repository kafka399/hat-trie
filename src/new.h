#include <iostream>
#include <map>
#include <sstream>
#include <cstdlib>

namespace vaszauskas {

using namespace std;

/**
 * A class that tracks memory allocations using an STL map.
 *
 * Be careful when using this class to track memory allocations in other
 * global objects. C++ makes no guarantees about the order of construction
 * for global objects, so there is no guarantee that the memory tracker
 * object will be constructed before any other global object.
 */
class MemoryTracker {

  private:
    struct Record {
        const char *file;
        int line;

        Record(const char *file, int line) : file(file), line(line) {}

        string str() {
            stringstream result;
            result << file << " at line " << line;
            return result.str();
        }
    };

    map<void *, Record> allocations;   // keep track of all memory allocations
    map<string, map<int, int> > data;  // reverse map to @a allocations

  public:
    MemoryTracker() {}

    ~MemoryTracker() {
        report();
    }

    void add(void *p, const char *file, int line) {
        // Add a record of the allocation to both maps.
        Record r = Record(file, line);
        allocations.insert(make_pair(p, r) );
        data[r.file][r.line]++;
    }

    void remove(void *p) {
        map<void *, Record>::iterator it = allocations.find(p);
        if (it != allocations.end() ) {
            // Guaranteed to find a corresponding entry in data.
            map<string, map<int, int> >::iterator sit;
            sit = data.find(it->second.file);

            // Found the corresponding entry in data; now find the internal
            // entry in the map inside sit.
            map<int, int>::iterator mit;
            mit = sit->second.find(it->second.line);

            // Found the internal entry; erase it if it is 0.
            if (--mit->second == 0) {
                sit->second.erase(mit);
            }
            allocations.erase(it);
        }
    }

    void report() {
        if (allocations.size() == 0) {
            return;
        }

        // Report memory leaks to stderr.
        cerr << "============" << endl;
        cerr << "MEMORY LEAKS" << endl;
        map<string, map<int, int> >::iterator it;
        map<int, int>::iterator mit;
        for (it = data.begin(); it != data.end(); ++it) {
            cerr << "  " << it->first << endl;
            for (mit = it->second.begin(); mit != it->second.end(); ++mit) {
                int leaks = mit->second;
                cerr << "    " << leaks << " leak";
                if (leaks > 1) {
                    cerr << "s";
                }
                cerr << " at line " << mit->first << endl;
            }
        }
    }
};

MemoryTracker _memory_tracker;

}  // namespace vaszauskas

/**
 * Replacement global operator new.
 *
 * @param size  Size of the memory chunk to allocate.
 * @param file  Filename the call to new originated from.
 * @param line  Line number the call to new originated from.
 * @return  A pointer to the newly allocated chunk of memory.
 *
 * Uses the global object _memory_tracker (defined in namespace
 * vaszauskas) to track memory allocations.
 */
void * operator new(size_t size, const char *file, int line) {
    void *p = malloc(size);
    vaszauskas::_memory_tracker.add(p, file, line);
    return p;
}

void * operator new[](size_t size, const char *file, int line) {
    return operator new(size, file, line);
}

/**
 * Replacement global operator delete.
 *
 * @param p  Pointer to a chunk of memory to free.
 *
 * Uses the global object _memory_tracker (defined in namespace
 * vaszauskas) to track memory allocations.
 */
void operator delete(void *p) throw() {
    vaszauskas::_memory_tracker.remove(p);
    free(p);
}

void operator delete[](void *p) throw() {
    operator delete(p);
}

#define new new(__FILE__, __LINE__)  // replace global new with ours

