// TODO
// what if bursting gives you a container that still has more elements than
// BURST_THRESHOLD? I think bursting this one lazily (on the next time you
// insert into this container) is acceptable.

#ifndef HAT_TRIE_H
#define HAT_TRIE_H

#include <bitset>
#include <exception>
#include <set>
#include <string>
#include <utility>

#include "array-hash.h"

using namespace std;

namespace stx {

template <int alphabet_size, int (*indexof)(char)>
class hat_trie;

// Exception class for bad index values.
class bad_index : public exception {
    virtual const char *what() const throw() {
        return "Invalid index value from indexof() function.";
    }
};

}

namespace {

// default value for hat_trie alphabet size
const int DEFAULT_ALPHABET_SIZE = 26;

/**
 * Default indexof function for hat_tries.
 */
inline int alphabet_index(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a';
    }
    return -1;
}

// --------------------------
// hat trie helper structures
// --------------------------

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_container;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_container {
    friend class stx::hat_trie<alphabet_size, indexof>;

  private:
    typedef hat_trie_node<alphabet_size, indexof> node;

  public:
    typedef stx::array_hash store_type;

    hat_trie_container(char ch = '\0');
    virtual ~hat_trie_container();

    bool contains(const string& s);
    void insert(const string& s);
    size_t size() const;

  private:
    char ch;
    bool word;
    node *parent;
    stx::array_hash store;
};

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node {
    friend class stx::hat_trie<alphabet_size, indexof>;

  private:
    typedef hat_trie_container<alphabet_size, indexof> container;
    typedef hat_trie_node<alphabet_size, indexof> node;

  public:
    hat_trie_node(char ch = '\0');
    ~hat_trie_node();

  public:
    char ch;
    void *children[alphabet_size];  // untyped pointers to children
    // To keep track of pointer types. The extra bit is an end-of-string flag.
    std::bitset<alphabet_size + 1> types;
    node *parent;
};

}  // unnamed namespace

namespace stx {

template <int alphabet_size = DEFAULT_ALPHABET_SIZE,
          int (*indexof)(char) = alphabet_index>
class hat_trie {
  private:
    typedef hat_trie_node<alphabet_size, indexof> node;
    typedef hat_trie_container<alphabet_size, indexof> container;

  public:
    hat_trie();
    virtual ~hat_trie();

    void insert(const string& s);
    void print(void *p, int type, const string& space = "") {
        if (type == CONTAINER_POINTER) {
            container *c = (container *)p;
            set<string>::iterator sit;
            if (c->ch != '\0') {
                cout << space << c->ch;
                if (c->word) {
                    cout << " ~";
                }
                cout << endl;
            }
            for (sit = c->container.begin(); sit != c->container.end(); ++sit) {
                assert(*sit != "");
                cout << space + "  " << *sit << " ~" << endl;
            }
        } else if (type == NODE_POINTER) {
            node *n = (node *)p;
            if (n->ch != '\0') {
                cout << space << n->ch;
                if (n->types[alphabet_size]) {
                    cout << " ~";
                }
                cout << endl;
            }
            for (int i = 0; i < alphabet_size; ++i) {
                if (n->children[i]) {
                    print(n->children[i], n->types[i], space + "  ");
                }
            }
        }
    }

  public:
    size_t _size;  // number of distinct elements in the trie
    void *root;    // root of the trie
    char type;     // pointer type of root

    // constant values for the hat trie
    enum { CONTAINER_POINTER, NODE_POINTER };
    enum { BUCKET_SIZE_THRESHOLD = 1024 };

    void init();
    int getindex(char ch) throw(bad_index);
    bool search(const string& s);
    bool search(const string& s, pair<void *, int> &p, size_t& pos);
    void burst(container *htc);
};

}  // namespace stx

namespace {

// ---------------------------------
// hat_trie_container implementation
// ---------------------------------

template <int alphabet_size, int (*indexof)(char)>
hat_trie_container<alphabet_size, indexof>::
hat_trie_container(char ch) : ch(ch), word(false), parent(NULL) {

}

template <int alphabet_size, int (*indexof)(char)>
hat_trie_container<alphabet_size, indexof>::
~hat_trie_container() {

}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
contains(const string& s) {
    return store.find(s.c_str());
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie_container<alphabet_size, indexof>::
insert(const string& s) {
    store.insert(s.c_str());
}

template <int alphabet_size, int (*indexof)(char)>
size_t hat_trie_container<alphabet_size, indexof>::
size() const {
    return store.size();
}

// ----------------------------
// hat_trie_node implementation
// ----------------------------

template <int alphabet_size, int (*indexof)(char)>
hat_trie_node<alphabet_size, indexof>::
hat_trie_node(char ch) : ch(ch), parent(NULL) {
    for (int i = 0; i < alphabet_size; ++i) {
        children[i] = NULL;
    }
}

template <int alphabet_size, int (*indexof)(char)>
hat_trie_node<alphabet_size, indexof>::
~hat_trie_node() {

}

}  // anonymous namespace

namespace stx {

// -----------------------
// hat_trie implementation
// -----------------------

template <int alphabet_size, int (*indexof)(char)>
hat_trie<alphabet_size, indexof>::hat_trie() {
    init();
}

template <int alphabet_size, int (*indexof)(char)>
hat_trie<alphabet_size, indexof>::~hat_trie() {
    if (type == CONTAINER_POINTER) { delete (container *)root; }
    else if (type == NODE_POINTER) { delete (node *)root; }
    root = NULL;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::init() {
    _size = 0;
    root = new container();
    type = CONTAINER_POINTER;
}

template <int alphabet_size, int (*indexof)(char)>
int hat_trie<alphabet_size, indexof>::getindex(char ch) throw(bad_index) {
    int result = indexof(ch);
    if (result < 0 || result >= alphabet_size) {
        throw bad_index();
    }
    return result;
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
search(const string& s) {
    pair<void *, int> p;
    size_t pos;
    return search(s, p, pos);
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
search(const string& s, pair<void *, int>& p, size_t& pos) {
    // Search for @a s in the tree.
    if (type == CONTAINER_POINTER) {
        container *htc = (container *)root;
        p = pair<void *, int>(root, CONTAINER_POINTER);
        pos = s.length();
        return htc->contains(s);

    } else if (type == NODE_POINTER) {
        node *n = (node *)root;
        void *v = NULL;
        pos = 0;
        // Traverse the trie until either @a s is used up, @a s is found in
        // the trie, or @a s is not found in the trie.
        while (pos < s.length()) {
            int index = getindex(s[pos]);
            v = n->children[index];
            if (v) {
                if (n->types[index] == NODE_POINTER) {
                    n = (node *)v;
                    //cout << "followed node path by " << s[pos] << endl;
                } else if (n->types[index] == CONTAINER_POINTER) {
                    //cout << "found container pointer at " << s[pos] << endl;
                    if (pos == s.length() - 1) {
                        // This node represents @a s.
                        p = pair<void *, int>(v, CONTAINER_POINTER);
                        ++pos;
                        //cout << "return here" << endl;
                        return ((container *)v)->word;
                    } else {
                        //cout << "found existing container" << endl;
                        // @a s should appear in the container represented by
                        // @a v.
                        p = pair<void *, int>(v, CONTAINER_POINTER);
                        if (pos == s.length()) {
                            return ((container *)v)->word;
                        } else {
                            return ((container *)v)->contains(s.substr(pos + 1));
                        }
                    }
                }
            } else {
                p = pair<void *, int>(n, NODE_POINTER);
                return false;
            }
            ++pos;
        }
        // If we get here, no container was found that could have held @a s,
        // meaning @a s should appear at node @a n in the trie. Return true
        // iff the end of string flag in @a n is set.
        p = pair<void *, int>(n, NODE_POINTER);
        return n->types[alphabet_size];
    }
    return false;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::insert(const string& s) {
    cout << "INSERTING " << s << endl;
    if (type == CONTAINER_POINTER) {
        // Insert into the container root points to.
        container *htc = (container *)root;
        htc->insert(s);
        if (htc->size() > BUCKET_SIZE_THRESHOLD) {
            burst(htc);
        }

    } else if (type == NODE_POINTER) {
        // Find the location in the trie that the new word should be inserted
        // into.
        pair<void *, int> p;
        size_t pos = 0;
        if (search(s, p, pos)) {
            //cout << "found " << s << " in the trie, so didn't insert" << endl;
        } else {
            // @a s wasn't found in the trie. Insert it.
            if (p.second == NODE_POINTER) {
                //cout << "p.second is node pointer" << endl;
                node *n = (node *)p.first;
                if (pos == s.length()) {
                    //cout << "pos == s.length()" << endl;
                    // Just set the node's end of word flag to true.
                    n->types.set(alphabet_size);
                } else {
                    //cout << "pos != s.length(): " << pos << endl;
                    // A new container needs to be made to accomodate @a s.
                    container *c = new container(s[pos]);
                    c->parent = n;
                    if (pos + 1 == s.length()) {
                        // The new container itself represents @a s.
                        c->word = true;
                    } else {
                        // Add the remainder of @a s to the new container.
                        c->insert(s.substr(pos + 1));
                    }
                    int index = getindex(s[pos]);
                    n->children[index] = c;
                    n->types[index] = CONTAINER_POINTER;
                }
            } else if (p.second == CONTAINER_POINTER) {
                //cout << "p.second is container pointer" << endl;
                // The word needs to be added to the trie.
                container *c = (container *)p.first;
                if (pos == s.length()) {
                    //cout << "pos == s.length()" << endl;
                    // Set the container's word flag to true.
                    c->word = true;
                } else {
                    //cout << "pos != s.length()" << endl;
                    // Insert the leftover part of @a s into the container.
                    c->insert(s.substr(pos + 1));
                    if (c->size() > BUCKET_SIZE_THRESHOLD) {
                        burst(c);
                    }
                }
            }
        }
    }
}

template <int alphabet_size, int (*indexof)(char)>
void
hat_trie<alphabet_size, indexof>::burst(container *htc) {
    cout << "BURSTING " << endl;
    // Construct new node.
    node *result = new node(htc->ch);
    result->types[alphabet_size] = htc->word;

    // Make a set of containers for the data in the old container and add them
    // to the new node.
    // TODO container::store_type::iterator it;
    array_hash::iterator it;
    for (it = htc->store.begin(); it != htc->store.end(); ++it) {
        //int index = getindex(it.first[0]);
        int index = getindex((*it)[0]);
        if (result->children[index] == NULL) {
            container *insertion = new container((*it)[0]);
            //insertion->word = length == 1;
            insertion->word == ((*it)[1] != '\0');
            insertion->parent = result;
            result->children[index] = insertion;
            result->types[index] = CONTAINER_POINTER;
        }
        if ((*it)[1] != '\0') {  // then the length is > 1
            //((container *)result->children[index])->insert(it->substr(1));
            ((container *)result->children[index])->insert((*it) + 1);
        }
    }

    // Position the new node in the tree.
    node *parent = htc->parent;
    result->parent = parent;
    if (parent) {
        int index = getindex(htc->ch);
        parent->children[index] = result;
        parent->types[index] = NODE_POINTER;
    } else {
        root = result;
        type = NODE_POINTER;
    }
    delete htc;
}

}  // namespace stx

#endif  // HAT_TRIE_H

