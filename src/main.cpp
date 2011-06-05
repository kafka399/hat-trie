#include <cassert>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>
#include <unistd.h>  // for sleep()
#include <vector>

#include <google/profiler.h>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "array_hash.h"
#include "hat_set.h"

using namespace std;
using namespace stx;

template <class T>
void print(const T &t, const string &space = "") {
    // use boost's foreach function! woot!
    foreach (const string &s, t) {
        cout << space + s << '\n';
    }
}

template <>
void print(const array_hash<string> &ah, const string &space) {
    array_hash<string>::iterator it;
    for (it = ah.begin(); it != ah.end(); ++it) {
        cout << space + *it << endl;
    }
}

void stl() {
    set<string> s;
    string reader;
    while (cin >> reader) {
        s.insert(reader);
    }
}

template <class A, class B>
void assert_equals(const A &a, const B &b) {
    set<string> x;
    set<string> y;

    typename A::iterator it = a.begin();
    while (it != a.end()) {
        x.insert(*it);
        ++it;
    }

    typename B::iterator bit = b.begin();
    while (bit != b.end()) {
        y.insert(*bit);
        ++bit;
    }

    if (x != y) {
        set<string>::iterator xit = x.begin();
        set<string>::iterator yit = y.begin();
        while (xit != x.end() && yit != y.end()) {
            if (*xit != *yit) {
                cout << *xit << " " << *yit << endl;
                assert(*xit == *yit);
            }
            ++xit;
            ++yit;
        }
    }
    assert(x == y);
}

void erase_test() {
    hat_set<string> ah;
    set<string> s;
    string reader;
    while (cin >> reader) {
        ah.insert(reader.c_str());
        s.insert(reader);
        cout << "INSERTED " << reader << endl;
    }
    vector<string> all(s.begin(), s.end());

    //assert_equals(s, ah);
    int count = 0;
    while (!s.empty()) {
        hat_set<string>::iterator it = ah.begin();
        for ( ; it != ah.end(); ++it) {
            if ((*it).length() == 0) {
                assert(false);
            }
            //cout << *it << endl;
        }
        //ah.trie.print();

        cout << "ERASING " << *ah.begin() << endl;
        //cout << "ERASED" << endl;
        //ah.trie.print();
        s.erase(*ah.begin());
        ah.erase(ah.begin());
        assert_equals(s, ah);

        if (++count == 10) {
            // Insert 5 random words from all.
            for (int i = 0; i < 5; ++i) {
                int r = rand() % all.size();
                cout << "INSERTING " << all[r] << endl;
                ah.insert(all[r]);
                s.insert(all[r]);
            }
            count = 0;
        }
    }
}

void mine_c() {
    hat_set<string> ht;

    // read entire file into main memory
    //FILE *f = fopen("/Users/chris/hat trie/inputs/distinct", "r");
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
    //ht.trie.print();

    while (!ht.empty()) {
        cout << "ERASING " << *ht.begin() << endl;
        ht.erase(*ht.begin());
        //ht.trie.print();
    }

    //sleep(1000);
    //print(ht);
}

void mine() {
    hat_set<string> ht;
    string reader;
    while (cin >> reader) {
        ht.insert(reader);
    }
}

void printsert(hat_set<string> &ht, const string& s) {
    cout << "INSERTING " << s << endl;
    ht.insert(s);
    ht.print();
}

void printrase(hat_set<string> &ht, const string& s) {
    cout << "ERASING " << s << endl;
    ht.erase(s);
    ht.print();
}

int main() {
    std::ios_base::sync_with_stdio(false);  // speed up reading from stdin

    ProfilerStart("profile/prof.prof");
    //stl();
    mine_c();
    //erase_test();
    ProfilerStop();
    return 0;
}

