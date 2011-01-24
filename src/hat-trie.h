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
        return "hat_trie: found unindexed character.";
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

    bool contains(const char *p);
    bool insert(const char *p);
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

    // accessors
    size_t size() const;

    // modifiers
    void insert(const string& s);

    void print(void *p, int type, const string& space = "") {
        if (type == CONTAINER_POINTER) {
            container *c = (container *)p;
            // TODO
            array_hash::iterator it;
            if (c->ch != '\0') {
                cout << space << c->ch;
                if (c->word) {
                    cout << " ~";
                }
                cout << endl;
            }
            for (it = c->store.begin(); it != c->store.end(); ++it) {
                //assert(*it != "");
                cout << space + "  " << *it << " ~" << endl;
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
    bool search(const char *& s, pair<void *, int> &p);
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
contains(const char *p) {
    if (*p == '\0') {
        return word;
    }
    return store.find(p);
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
insert(const char *p) {
    return store.insert(p);
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
    const char *ps;
    pair<void *, int> p;
    return search(ps, p);
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
search(const char *& s, pair<void *, int>& p) {
    // Search for a s in the tree.
    if (type == CONTAINER_POINTER) {
        container *htc = (container *)root;
        p = pair<void *, int>(root, CONTAINER_POINTER);
        bool b = htc->contains(s);
        s = '\0';
        return b;

    } else if (type == NODE_POINTER) {
        // Traverse the trie until either a s is used up, a is is found
        // in the trie, or s cannot be in the trie.
        node *n = (node *)root;
        void *v = NULL;
        while (*s) {
            int index = getindex(*s);
            v = n->children[index];
            if (v) {
                if (n->types[index] == NODE_POINTER) {
                    // Keep moving down the trie structure.
                    n = (node *)v;
                } else if (n->types[index] == CONTAINER_POINTER) {
                    // s should appear in the container v. If it
                    // doesn't, it's not in the trie.
                    p = pair<void *, int>(v, CONTAINER_POINTER);
                    return ((container *)v)->contains(s + 1);
                }
            } else {
                p = pair<void *, int>(n, NODE_POINTER);
                return false;
            }
            ++s;
        }
        // If we get here, no container was found that could have held
        // s, meaning node n represents s in the trie. Return true if
        // the end of word flag in n is set.
        p = pair<void *, int>(n, NODE_POINTER);
        return n->types[alphabet_size];
    }
    return false;
}

template <int alphabet_size, int (*indexof)(char)>
size_t hat_trie<alphabet_size, indexof>::size() const {
    return _size;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::insert(const string& s) {
    if (type == CONTAINER_POINTER) {
        // Insert into the container root points to.
        container *htc = (container *)root;
        if (htc->insert(s.c_str())) {
            ++_size;
            if (htc->size() > BUCKET_SIZE_THRESHOLD) {
                burst(htc);
            }
        }

    } else if (type == NODE_POINTER) {
        // Find the location in the trie that the new word should be inserted
        // into.
        const char *pos = s.c_str();
        pair<void *, int> p;
        if (search(pos, p)) {
//            cout << "found " << s << " in the trie, so didn't insert" << endl;
        } else {
            // @a s wasn't found in the trie. Insert it.
            if (p.second == NODE_POINTER) {
                node *n = (node *)p.first;
                if (*pos == '\0') {
                    // Just set the node's end of word flag to true.
                    n->types.set(alphabet_size);
                } else {
                    // A new container needs to be made to accomodate @a s.
                    container *c = new container(*pos);
                    c->parent = n;
                    if (*(pos + 1) == '\0') {
                        // The new container itself represents @a s.
                        c->word = true;
                    } else {
                        // Add the remainder of @a s to the new container.
                        if (c->insert(pos + 1)) {
                            ++_size;
                            if (c->size() > BUCKET_SIZE_THRESHOLD) {
                                burst(c);
                            }
                        }
                    }
                    //int index = getindex(s[pos]);
                    int index = getindex(*pos);
                    n->children[index] = c;
                    n->types[index] = CONTAINER_POINTER;
                }
            } else if (p.second == CONTAINER_POINTER) {
                // The word needs to be added to the trie.
                container *c = (container *)p.first;
                if (*pos == '\0') {
                    // Set the container's word flag to true.
                    c->word = true;
                } else {
                    // Insert the leftover part of @a s into the container.
                    if (c->insert(pos + 1)) {
                        ++_size;
                        if (c->size() > BUCKET_SIZE_THRESHOLD) {
                            burst(c);
                        }
                    }
                }
            }
        }

//      container *c = NULL;
//      if (p.second == NODE_POINTER) {
//          node *n = (node *)p.first;
//          if (*pos == '\0') {
//              // Set the node's end of word flag to true.
//              n->types.set(alphabet_size);
//          } else {
//              // A new container needs to be made to accomodate @a s.
//              c = new container(*pos);
//              c->parent = n;
//              int index = getindex(*pos);
//              n->children[index] = c;
//              n->types[index] = CONTAINER_POINTER;
//              ++pos;
//            //if (*pos == '\0') {
//            //    // The new container represents @a s.
//            //    c->word = true;
//            //}
//          }
//      } else if (p.second == CONTAINER_POINTER) {
//          // The word needs to be added to a container already in the trie.
//          c = (container *)p.first;
//      }
//      if (c) {

//      }
    }
}

template <int alphabet_size, int (*indexof)(char)>
void
hat_trie<alphabet_size, indexof>::burst(container *htc) {
//    cout << "BURSTING " << endl;
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
            insertion->word = ((*it)[1] != '\0');
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

