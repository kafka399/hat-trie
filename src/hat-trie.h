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
#include "hat-trie-node.h"

using namespace std;

namespace stx {

/**
 * Exception class for unindexed characters.
 *
 * This exception is thrown when a hat_trie encounters an unindexed
 * character. indexof() returns a value out of range (less than 0 or
 * greater than @a alphabet_size) for unindexed characters.
 */
class unindexed_character : public exception {
    virtual const char *what() const throw() {
        return "hat_trie: found unindexed character.";
    }
};

/// default value for hat_trie alphabet size
const int DEFAULT_ALPHABET_SIZE = 26;

/// default indexof function for hat_tries
inline int alphabet_index(char ch) {
    return ch - 'a';
}

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
    bool contains(const string& s);
    size_t size() const;

    // modifiers
    bool insert(const string& s);

    void print() {
        print(root, type);
    }
    void print(void *p, int type, const string& space = "") {
        if (type == CONTAINER_POINTER) {
            container *c = (container *)p;
            // TODO
            array_hash::iterator it;
            if (c->ch() != '\0') {
                cout << space << c->ch();
                if (c->word()) {
                    cout << " ~";
                }
                cout << endl;
            }
            for (it = c->store.begin(); it != c->store.end(); ++it) {
                cout << space + "  " << *it << " ~" << endl;
            }
        } else if (type == NODE_POINTER) {
            node *n = (node *)p;
            if (n->ch() != '\0') {
                cout << space << n->ch();
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
    bool insert(container *htc, const char *s);
    void burst(container *htc);
};

// -----------------------
// hat_trie implementation
// -----------------------

/**
 * Default constructor.
 */
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

/**
 * Searches for a word in the trie.
 *
 * @param s  word to search for
 *
 * @return  true if @a s is in the trie, false otherwise.
 * @throws unindexed_character  if a character in @a s is not indexed
 *                              by @a indexof()
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
contains(const string& s) {
    const char *ps = s.c_str();
    pair<void *, int> p;
    return search(ps, p);
}

/**
 * Gets the number of distinct elements in the trie.
 *
 * @return  size of the trie
 */
template <int alphabet_size, int (*indexof)(char)>
size_t hat_trie<alphabet_size, indexof>::size() const {
    return _size;
}

/**
 * Inserts a word into the trie.
 *
 * @param s  word to insert
 *
 * @return  false if @a s is already in the trie, true otherwise
 * @throws unindexed_character  if a character in @a s is not indexed
 *                              by @a indexof()
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
insert(const string& s) {
    if (type == CONTAINER_POINTER) {
        // Insert into the container root points to.
        return insert((container *)root, s.c_str());

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
                    ((container *)(p.first))->set_word(true);
                }
            } else {
                // s was not found in the trie's structure. Either make a
                // new container for it or insert it into an already
                // existing container.
                container *c = NULL;
                if (p.second == NODE_POINTER) {
                    // Make a new container for s.
                    node *n = (node *)p.first;
                    int index = get_index(*pos);
                    c = new container(*pos);

                    // Insert the new container into the trie structure.
                    c->parent = n;
                    n->children[index] = c;
                    n->types[index] = CONTAINER_POINTER;
                    ++pos;
                } else if (p.second == CONTAINER_POINTER) {
                    c = (container *)p.first;
                }

                // Insert s into the container.
                return insert(c, pos);
            }
        }
    }
    return false; // unreachable code
}

/**
 * Initializes all the fields in a hat_trie as if it had just been
 * created.
 */
template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::init() {
    _size = 0;
    root = new container();
    type = CONTAINER_POINTER;
}

/**
 * Gets the index of a character.
 *
 * @param ch  character to index
 *
 * @return  index of @a ch
 * @throws unindexed_character  if @a ch is not indexed by @a indexof()
 */
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
    // Search for a s in the trie.
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
bool hat_trie<alphabet_size, indexof>::
insert(container *htc, const char *s) {
    if (htc->insert(s)) {
        ++_size;
        if (htc->size() > BURST_THRESHOLD) {
            burst(htc);
        }
        return true;
    }
    return false;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
burst(container *htc) {
    // Construct new node.
    node *result = new node(htc->ch());
    result->set_word(htc->word());

    // Make a set of containers for the data in the old container and
    // add them to the new node.
    // TODO container::store_type::iterator it;
    array_hash::iterator it;
    for (it = htc->store.begin(); it != htc->store.end(); ++it) {
        int index = get_index((*it)[0]);
        if (result->children[index] == NULL) {
            container *insertion = new container((*it)[0]);
            insertion->set_word(((*it)[1] == '\0'));
            insertion->parent = result;
            result->children[index] = insertion;
            result->types[index] = CONTAINER_POINTER;
        }
        if ((*it)[1] != '\0') {  // then the length is > 1
            ((container *)result->children[index])->insert((*it) + 1);
        }
    }

    // Position the new node in the trie.
    node *parent = htc->parent;
    result->parent = parent;
    if (parent) {
        int index = get_index(htc->ch());
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

