#include <cassert>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>
#include <unistd.h>  // for sleep()

#include <google/profiler.h>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "array-hash.h"
#include "hat-trie.h"

using namespace std;
using namespace stx;

template <class T>
void print(const T &t) {
    // use boost's foreach function! woot!
    foreach (const string &s, t) {
        cout << s << '\n';
    }
}

template <>
void print(const array_hash<string> &ah) {
    array_hash<string>::iterator it;
    for (it = ah.begin(); it != ah.end(); ++it) {
        cout << *it << endl;
    }
}

void stl() {
    set<string> s;
    string reader;
    while (cin >> reader) {
        s.insert(reader);
    }
}

void mine_c() {
    hat_trie<string> ht;
    //array_hash<string> ht;

    // read entire file into main memory
    FILE *f = stdin;
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    char *data = new char[size];
    fseek(f, 0, SEEK_SET);
    fread(data, 1, size, f);
    fclose(f);

    // scan through the file and insert all the words into the
    // trie
    char *start = data;
    for (int i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            data[i] = '\0';
            ht.insert(start);
            start = data + i + 1;
        }
    }
    delete [] data;
    //print(ht);
}

void mine() {
    //hat_trie ht;
    string reader;
    while (cin >> reader) {
        //ht.insert(reader);
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);  // speed up reading from stdin

    //ProfilerStart("profile/prof.prof");
    //stl();
    //mine();
    mine_c();
    //ProfilerStop();

    return 0;
}

