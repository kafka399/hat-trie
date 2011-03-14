/*
 * Copyright 2010-2011 Chris Vaszauskas and Tyler Richard
 *
 * This file is part of a HAT-trie implementation.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    typedef hat_trie_node_base<alphabet_size, indexof> node_base;

    struct node_pointer {
        unsigned char type;
        node_base *p;

        node_pointer(unsigned char type = 0, node_base *p = NULL) :
                type(type), p(p) {

        }
    };

  public:
	class iterator;

    // constructors and destructors
    hat_trie();
    virtual ~hat_trie();

    // accessors
    bool contains(const string& s) const;
    size_t size() const;
    void print() const { print(root); }

    // modifiers
    bool insert(const string& s);

    // iterators
    iterator begin() const;
    iterator end() const;

    class iterator : std::iterator<bidirectional_iterator_tag, node_base> {
        friend class hat_trie;

      public:
        iterator();
        iterator(const iterator& rhs);

        iterator operator++(int);
        iterator& operator++();
        iterator operator--(int);
        iterator& operator--();

        string operator*() const;
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);
        iterator& operator=(const iterator &rhs);

      private:
        // current location and node type of that location
        node_pointer n;
        typename container::store_type::iterator it;

    };

  private:
    node_pointer root;  // pointer to the root of the trie
    size_t _size;     // number of distinct elements in the trie

    // types node_base * values could point to
    enum { CONTAINER_POINTER = 0, NODE_POINTER = 1 };

    // containers are burst after their size crosses this threshold
    enum { BURST_THRESHOLD = 1024 };

    void init();

    // accessors
    int get_index(char ch) const throw(unindexed_character);
    bool search(const char *& s, node_pointer &n) const;
    void print(const node_pointer &n, const string& space = "") const;

    // modifiers
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
    delete root.p;
    root.p = NULL;
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
contains(const string& s) const {
    const char *ps = s.c_str();
    node_pointer n;
    return search(ps, n);
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
    if (root.type == CONTAINER_POINTER) {
        // Insert into the container root points to.
        return insert((container *)root.p, s.c_str());

    } else if (root.type == NODE_POINTER) {
        // Search for s in the trie.
        const char *pos = s.c_str();
        node_pointer n;
        if (search(pos, n) == false) {
            // Was s found in the structure of the trie?
            if (*pos == '\0') {
                // s was found in the trie's structure. Mark its location
                // as the end of a word.
                n.p->set_word(true);
            } else {
                // s was not found in the trie's structure. Either make a
                // new container for it or insert it into an already
                // existing container.
                container *c = NULL;
                if (n.type == NODE_POINTER) {
                    // Make a new container for s.
                    node *p = (node *)n.p;
                    int index = get_index(*pos);
                    c = new container(*pos);

                    // Insert the new container into the trie structure.
                    c->parent = p;
                    p->children[index] = c;
                    p->types[index] = CONTAINER_POINTER;
                    ++pos;
                } else if (n.type == CONTAINER_POINTER) {
                    c = (container *)n.p;
                }

                // Insert s into the container.
                return insert(c, pos);
            }
        }
    }
    return false;  // s was found in the trie
}

template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator
hat_trie<alphabet_size, indexof>::begin() const {

}

/**
 * Initializes all the fields in a hat_trie as if it had just been
 * created.
 */
template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::init() {
    _size = 0;
    root.p = new container();
    root.type = CONTAINER_POINTER;
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
get_index(char ch) const throw(unindexed_character) {
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
search(const char *& s, node_pointer &n) const {
    // Search for a s in the trie.
    if (root.type == CONTAINER_POINTER) {
        container *htc = (container *)root.p;
        n = root;
        bool b = htc->contains(s);
        return b;

    } else if (root.type == NODE_POINTER) {
        // Traverse the trie until either a s is used up, a is is found
        // in the trie, or s cannot be in the trie.
        node *p = (node *)root.p;
        node_base *v = NULL;
        while (*s) {
            int index = get_index(*s);
            v = p->children[index];
            if (v) {
                ++s;
                if (p->types[index] == NODE_POINTER) {
                    // Keep moving down the trie structure.
                    p = (node *)v;
                } else if (p->types[index] == CONTAINER_POINTER) {
                    // s should appear in the container v. If it
                    // doesn't, it's not in the trie.
                    n = node_pointer(CONTAINER_POINTER, v);
                    return ((container *)v)->contains(s);
                }
            } else {
                n = node_pointer(NODE_POINTER, p);
                return false;
            }
        }

        // If we get here, no container was found that could have held
        // s, meaning node n represents s in the trie. Return true if
        // the end of word flag in n is set.
        n = node_pointer(NODE_POINTER, p);
        return p->is_word();
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
    result->set_word(htc->is_word());

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
        root.p = result;
        root.type = NODE_POINTER;
    }
    delete htc;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
print(const node_pointer &n, const string& space) const {
    if (root.type == CONTAINER_POINTER) {
        container *c = (container *)n.p;
        // TODO
        array_hash::iterator it;
        if (c->ch() != '\0') {
            cout << space << c->ch();
            if (c->is_word()) {
                cout << " ~";
            }
            cout << endl;
        }
        for (it = c->store.begin(); it != c->store.end(); ++it) {
            cout << space + "  " << *it << " ~" << endl;
        }
    } else if (root.type == NODE_POINTER) {
        node *p = (node *)n.p;
        if (p->ch() != '\0') {
            cout << space << p->ch();
            if (p->types[alphabet_size]) {
                cout << " ~";
            }
            cout << endl;
        }
        for (int i = 0; i < alphabet_size; ++i) {
            if (p->children[i]) {
                print(node_pointer(p->types[i], p->children[i]), space + " ");
            }
        }
    }
}


template <int alphabet_size, int (*indexof)(char)>
hat_trie<alphabet_size, indexof>::
iterator::iterator() {

}

template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator&
hat_trie<alphabet_size, indexof>::
iterator::operator++() {

}

template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator&
hat_trie<alphabet_size, indexof>::
iterator::operator--() {

}

template <int alphabet_size, int (*indexof)(char)>
string hat_trie<alphabet_size, indexof>::
iterator::operator*() const {
    return *it;
    if (n.type == CONTAINER_POINTER) {
        return *it;
    } else if (n.type == NODE_POINTER) {
        return *it;
    }
}


}  // namespace stx

#endif  // HAT_TRIE_H

