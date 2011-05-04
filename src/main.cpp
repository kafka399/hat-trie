#include <cstdio>
#include <cassert>
#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <time.h>
#include <unistd.h>  // for sleep()
#include <iomanip>

#include <google/profiler.h>

#include "array-hash.h"
#include "hat-trie.h"

using namespace std;
using namespace stx;

template <class T>
void print(const T &t) {
    typename T::iterator it;
    for (it = t.begin(); it != t.end(); ++it) {
        cout << *it << endl;
    }
}

int main(int argc, char **args) {
    std::ios_base::sync_with_stdio(false);  // speed up reading from stdin

    //ProfilerStart("profile/prof.prof");
    hat_trie ht;

//    string reader;
//    while (cin >> reader) {
//        ht.insert(reader);
//    }

    // read entire file into main memory
    FILE *f = fopen(args[1], "r");
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    char *data = new char[size];
    fseek(f, 0, SEEK_SET);
    fread(data, 1, size, f);
    fclose(f);
    char *start = data;
    for (int i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            data[i] = '\0';
            ht.insert(start);
            start = data + i + 1;
        }
    }

    //sleep(1000);

    //print(ht);
    //ProfilerStop();

    return 0;
}
