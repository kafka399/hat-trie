// TODO
// what if bursting gives you a container that still has more elements than
// BURST_THRESHOLD? I think bursting this one lazily (on the next time you
// insert into this container) is acceptable. No container will have more
// values in it than BURST_THRESHOLD + 1.
//   NO! it accumulates!

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

/**
 * Exception class for unindexed characters.
 *
 * This exception is thrown when a hat_trie encounters an
 * unindexed character. The indexof() function returns -1 for
 * unindexed characters.
 */
class unindexed_character : public exception {
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

    // accessors
    bool contains(const char *p) const;
    size_t size() const;

    // modifiers
    bool insert(const char *p);

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

    // accessors
    bool is_word() const;

    // modifiers
    void set_word(bool b);

  public:
    char ch;
    void *children[alphabet_size];  // untyped pointers to children
    // To keep track of pointer types. The extra bit is an end-of-string flag.
    std::bitset<alphabet_size + 1> types;
    node *parent;
};

}  // unnamed namespace

namespace stx {

/**
 * Trie-based data structure for managing sorted strings.
 */
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
    bool search(const string& s);
    size_t size() const;

    // modifiers
    void insert(const string& s);

    void print() {
        print(root, type);
    }
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


  private:
    size_t _size;  // number of distinct elements in the trie
    void *root;    // root of the trie
    char type;     // pointer type of root

    // constant values for the hat trie
    enum { CONTAINER_POINTER, NODE_POINTER };
    enum { BURST_THRESHOLD = 1024 };

    void init();
    int get_index(char ch) throw(unindexed_character);
    bool search(const char *& s, pair<void *, int> &p);
    void insert(container *htc, const char *s);
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
contains(const char *p) const {
    if (*p == '\0') {
        return word;
    }
    return store.find(p);
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
insert(const char *p) {
    if (*p == '\0') {
        bool b = word;
        word = true;
        return !b;
    }
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

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_node<alphabet_size, indexof>::
is_word() const {
    return types[alphabet_size];
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie_node<alphabet_size, indexof>::
set_word(bool val) {
    types[alphabet_size] = val;
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
void hat_trie<alphabet_size, indexof>::
insert(const string& s) {
    if (type == CONTAINER_POINTER) {
        // Insert into the container root points to.
        //container *htc = (container *)root;
        insert((container *)root, s.c_str());

    } else if (type == NODE_POINTER) {
        // Search for s in the trie.
        const char *pos = s.c_str();
        pair<void *, int> p;
        if (search(pos, p) == false) {
            // Was s found in the structure of the trie?
            if (*pos == '\0') {
                // s was found in the trie's structure. Mark its location
                // as the end of a word.
                if (p.second == NODE_POINTER) {
                    ((node *)(p.first))->set_word(true);
                } else if (p.second == CONTAINER_POINTER) {
                    ((container *)(p.first))->word = true;
                }
            } else {
                // s was not found in the trie's structure. Either make a
                // new container for it or insert it into an already
                // existing container.
                container *c = NULL;
                if (p.second == NODE_POINTER) {
                    // Make a new container for s.
                    node *n = (node *)p.first;
                    //cout << 1 << endl;
                    int index = get_index(*pos);
                    c = new container(*pos);
                    for (int i = 0; i < 2; ++i) {
                        assert(c->store.data[i] == NULL);
                    }
                    c->parent = n;
                    n->children[index] = c;
                    n->types[index] = CONTAINER_POINTER;
                    ++pos;
                } else if (p.second == CONTAINER_POINTER) {
                    c = (container *)p.first;
                }

                // Insert s into the container.
                insert(c, pos);
            }
        }
    }
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
search(const string& s) {
    const char *ps = s.c_str();
    pair<void *, int> p;
    return search(ps, p);
}

template <int alphabet_size, int (*indexof)(char)>
size_t hat_trie<alphabet_size, indexof>::size() const {
    return _size;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::init() {
    _size = 0;
    root = new container();
    type = CONTAINER_POINTER;
}

template <int alphabet_size, int (*indexof)(char)>
int hat_trie<alphabet_size, indexof>::
get_index(char ch) throw(unindexed_character) {
    int result = indexof(ch);
    if (result < 0 || result >= alphabet_size) {
        throw unindexed_character();
    }
    return result;
}

/**
 * Searches for @a s in the trie, returning statistics about its position.
 *
 * @param s  string to search for. If @a *s = \0, @a s is in the trie
 *           part of this data structure. If not, @a s is in a
 *           container.
 * @param p  stores the position of @a s. @a first is a pointer to the
 *           node or container for @a s, and @a second is the pointer
 *           type
 *
 * @return  true if @a s is found in the trie, false otherwise
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
search(const char *& s, pair<void *, int>& p) {
    // Search for a s in the tree.
    if (type == CONTAINER_POINTER) {
        container *htc = (container *)root;
        p = pair<void *, int>(root, CONTAINER_POINTER);
        bool b = htc->contains(s);
        return b;

    } else if (type == NODE_POINTER) {
        // Traverse the trie until either a s is used up, a is is found
        // in the trie, or s cannot be in the trie.
        node *n = (node *)root;
        void *v = NULL;
        while (*s) {
            int index = get_index(*s);
            v = n->children[index];
            if (v) {
                ++s;
                if (n->types[index] == NODE_POINTER) {
                    // Keep moving down the trie structure.
                    n = (node *)v;
                } else if (n->types[index] == CONTAINER_POINTER) {
                    // s should appear in the container v. If it
                    // doesn't, it's not in the trie.
                    p = pair<void *, int>(v, CONTAINER_POINTER);
                    return ((container *)v)->contains(s);
                }
            } else {
                p = pair<void *, int>(n, NODE_POINTER);
                return false;
            }
        }

        // If we get here, no container was found that could have held
        // s, meaning node n represents s in the trie. Return true if
        // the end of word flag in n is set.
        p = pair<void *, int>(n, NODE_POINTER);
        return n->is_word();
    }
    return false;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
insert(container *htc, const char *s) {
    if (htc->insert(s)) {
        ++_size;
        if (htc->size() > BURST_THRESHOLD) {
            burst(htc);
        }
    }
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
burst(container *htc) {
    // Construct new node.
    node *result = new node(htc->ch);
    result->set_word(htc->word);

    // Make a set of containers for the data in the old container and
    // add them to the new node.
    // TODO container::store_type::iterator it;
    array_hash::iterator it;
    for (it = htc->store.begin(); it != htc->store.end(); ++it) {
        //int index = get_index(it.first[0]);
        int index = get_index((*it)[0]);
        assert(strlen(*it) > 0);
        if (result->children[index] == NULL) {
            container *insertion = new container((*it)[0]);
            insertion->word = ((*it)[1] == '\0');
            insertion->parent = result;
            result->children[index] = insertion;
            result->types[index] = CONTAINER_POINTER;
        }
        if ((*it)[1] != '\0') {  // then the length is > 1
            ((container *)result->children[index])->insert((*it) + 1);
        }
    }

    // Position the new node in the tree.
    node *parent = htc->parent;
    result->parent = parent;
    if (parent) {
        int index = get_index(htc->ch);
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

