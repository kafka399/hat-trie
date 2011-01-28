#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <unistd.h>  // for sleep()

#include "array-hash.h"
#include "hat-trie.h"

using namespace std;
using namespace stx;

int indexof(char ch) {
    if (ch - 'a' < 26 && ch - 'a' >= 0) {
        return ch - 'a';
    }
    if (ch == '\'') return 26;
    cout << "UNINDEXED CHAR: " << ch << endl;
    return -1;
}

string trim(string s) {
    string result;
    for (size_t i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
        if (indexof(s[i]) != -1) {
            result += s[i];
        }
    }
    return result;
}

bool test(const string& s, hat_trie<27, indexof>& ht, const set<string>& st) {
    cout << "TESTING " << s << ": " << flush;
    bool a;
    try { a = ht.search(s); }
    catch (bad_index) { a = false; }
    bool b = st.find(s) != st.end();
    cout << a << " " << b << endl;
    return a == b;
}

void print(const pair<const char *, uint16_t>& p) {
    for (int i = 0; i < p.second; ++i) {
        cout << p.first[i];
    }
    cout << endl;
}

template <class T>
void print(T& t) {
    typename T::iterator it;
    for (it = t.begin(); it != t.end(); ++it) {
        //print(*it);
        cout << *it << endl;
    }
}

template <class A>
bool compare(const A& a, const set<string>& s) {
    set<string>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        if (a.find(it->c_str()) == false) {
            return false;
        }
    }
    return s.size() == a.size();
}

//timer code
clock_t START_TIME = 1, STOP_TIME = 1;
void timeStart() {
	START_TIME = clock() * CLOCKS_PER_SEC;
}
void timeStop() {
	STOP_TIME = clock() * CLOCKS_PER_SEC;
}
double timeReport() {
	double res =  (STOP_TIME - START_TIME)/1000.0;
	cout << "Timer logged " << res << " ms."  << endl;
	return res;
}

int main() {
    std::ios_base::sync_with_stdio(false);  // speed up reading from stdin
    hat_trie<27, indexof> ht;
    string reader;
    set<string> s;
    array_hash ah;
    while (cin >> reader) {
        ht.insert(reader);
        //s.insert(reader);
        //ah.insert(reader.c_str());
    }

    return 0;
}

