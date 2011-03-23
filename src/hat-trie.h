/*
 * Copyright 2010-2011 Chris Vaszauskas and Tyler Richard
 *
 * This file is part of a HAT-trie implementation following the paper
 * entitled "HAT-trie: A Cache-concious Trie-based Data Structure for
 * Strings" by Nikolas Askitis and Ranjan Sinha.
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
// TODO make it really hard to use a container that isn't an array hash
// TODO documentation that limits string length to 65k characters
// TODO store the empty string at root
// TODO visual studio compatibility
// TODO make array_hash find function return an iterator
// TODO is unindexed_character really necessary?

#ifndef HAT_TRIE_H
#define HAT_TRIE_H

#include <bitset>
#include <string>
#include <utility>
#include <vector>

#include "array-hash.h"
#include "hat-trie-node.h"

namespace stx {

/**
 * Trie-based data structure for managing sorted strings.
 *
 * In the context of this data structure, "alphabet" refers to the
 * set of characters that can appear in a word. This data structure
 * requires that this alphabet is well-defined, meaning you must know
 * exactly what characters your strings can contain beforehand.
 *
 * // TODO describe why this limitation is necessary.
 *
 * @param alphabet_size
 *      number of distinct characters in the alphabet
 * @param indexof
 *      indexer function that maps characters to their position in
 *      the alphabet. By default, this function accepts alphanumeric
 *      characters
 */
template <int alphabet_size = HT_DEFAULT_ALPHABET_SIZE,
          int (*indexof)(char) = ht_alphanumeric_index>
class hat_trie {

  private:
    typedef hat_trie_node<alphabet_size, indexof> node;
    typedef hat_trie_container<alphabet_size, indexof> container;
    typedef hat_trie_node_base<alphabet_size, indexof> node_base;

    // pairs node_base pointers with their type (whether they point to
    // nodes or containers)
    class node_pointer {

      public:
        unsigned char type;
        node_base *pointer;

        node_pointer(unsigned char type = 0, node_base *pointer = NULL) :
                type(type), pointer(pointer) { }

        // comparison operators
        bool operator==(const node_pointer &rhs) {
            return pointer == rhs.pointer;
        }
        bool operator!=(const node_pointer &rhs) {
            return !operator==(rhs);
        }
    };

  public:
	class iterator;

    // constructors and destructors
    hat_trie();
    virtual ~hat_trie();

    // accessors
    bool contains(const std::string &s) const;
    size_t size() const;
    void print() const { print(root); }

    // modifiers
    bool insert(const std::string &s);

    // iterators
    iterator begin() const;
    iterator end() const;

    class iterator : std::iterator<std::bidirectional_iterator_tag, node_base> {
        friend class hat_trie;

      public:
        iterator(const node_pointer &n = node_pointer()) : n(n) { }
        iterator(const iterator &rhs);

        iterator operator++(int);
        iterator &operator++();
        iterator operator--(int);
        iterator &operator--();

        std::string operator*() const;
        bool operator==(const iterator &rhs);
        bool operator!=(const iterator &rhs);
        iterator &operator=(const iterator &rhs);

      private:
        // current location and node type of that location
        node_pointer n;

        // iterator over the elements in the current container
        typename container::store_type::iterator it;

        // caches the word as we travel down the trie
        std::string word;

        // caches our location in the hierarchy of the trie
        std::vector<int> pos;

    };

  private:
    node *root;  // pointer to the root of the trie
    size_t _size;  // number of distinct elements in the trie

    // types node_base values could point to. This is stored in
    // one bit, so the only valid values are 0 and 1
    enum { CONTAINER_POINTER = 0, NODE_POINTER = 1 };

    // containers are burst after their size crosses this threshold
    // MUST be <= 32,768
    enum { BURST_THRESHOLD = 8192 };

    void init();

    // accessors
    bool search(const char * &s, node_pointer &n) const;
    void print(const node_pointer &n, const std::string &space = "") const;
    static node_pointer next_word(node_pointer);

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
    delete root;
    root = NULL;
}

/**
 * Searches for a word in the trie.
 *
 * @param s  word to search for
 *
 * @return  true if @a s is in the trie, false otherwise.
 * @throws unindexed_character
 *      if a character in @a s is not indexed by @a indexof()
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
contains(const std::string &s) const {
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
size_t hat_trie<alphabet_size, indexof>::
size() const {
    return _size;
}

/**
 * Inserts a word into the trie.
 *
 * @param s  word to insert
 *
 * @return  false if @a s is already in the trie, true otherwise
 * @throws unindexed_character
 *      if a character in @a s is not indexed by @a indexof()
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
insert(const std::string &s) {
    // Search for s in the trie.
    const char *pos = s.c_str();
    node_pointer n;
    if (search(pos, n) == false) {
        // Was s found in the structure of the trie?
        if (*pos == '\0') {
            // s was found in the trie's structure. Mark its location
            // as the end of a word.
            n.pointer->set_word(true);

        } else {
            // s was not found in the trie's structure. Either make a
            // new container for it or insert it into an already
            // existing container.
            container *c = NULL;
            if (n.type == NODE_POINTER) {
                // Make a new container for s.
                node *p = (node *)n.pointer;
                int index = ht_get_index<alphabet_size, indexof>(*pos);
                c = new container(*pos);

                // Insert the new container into the trie structure.
                c->parent = p;
                p->children[index] = c;
                p->types[index] = CONTAINER_POINTER;
                ++pos;
            } else if (n.type == CONTAINER_POINTER) {
                c = (container *)n.pointer;
            }

            // Insert s into the container.
            return insert(c, pos);
        }
    }

    return false;  // s was found in the trie
}

/**
 * Gets an iterator to the first element in the trie.
 *
 * If there are no elements in the trie, the iterator pointing to
 * trie.end() is returned.
 *
 * @return  iterator to the first element in the trie
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator
hat_trie<alphabet_size, indexof>::
begin() const {
    if (size() == 0) {
        return end();
    }
    return iterator(next_word(root));
}

/**
 * Gets an iterator to one past the last element in the trie.
 *
 * @return iterator to one past the last element in the trie.
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator
hat_trie<alphabet_size, indexof>::
end() const {
    return iterator();
}

/**
 * Initializes all the fields in a hat_trie as if it had just been
 * created.
 */
template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::init() {
    _size = 0;
    root = new node();
}

/**
 * Searches for @a s in the trie, returning statistics about its position.
 *
 * @param s  string to search for. After this function completes, if
 *           <code>*s = '\0'</code>, @a s is in the trie part of this
 *           data structure. If not, @a s is either completed in a
 *           container or is not in the trie at all.
 * @param p  after this function completes, @a p stores a pointer to
 *           either the node or container for @a s
 *
 * @return  true if @a s is found, false otherwise
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
search(const char *&s, node_pointer &n) const {
    // Search for a s in the trie.
    // Traverse the trie until either a s is used up, a is is found
    // in the trie, or s cannot be in the trie.
    node *p = root;
    node_base *v = NULL;
    while (*s) {
        int index = ht_get_index<alphabet_size, indexof>(*s);
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

/**
 * Inserts a word into a container.
 *
 * If the insertion overflows the burst threshold, the container
 * is burst.
 *
 * @param htc  container to insert into
 * @param s    word to insert
 *
 * @return
 *      true if @a s is successfully inserted into @a htc, false
 *      otherwise
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
insert(container *htc, const char *s) {
    // Try to insert s into the container.
    if (htc->insert(s)) {
        ++_size;
        if (htc->size() > BURST_THRESHOLD) {
            // The container has too many strings in it; burst the
            // container into a node.
            burst(htc);
        }
        return true;
    }
    return false;
}

/**
 * Bursts a container into a node with containers underneath it.
 *
 * If this container contains the words tan, tree, and trust, it
 * will be split into a node with two containers underneath it. The
 * structure will look like this:
 *
 *   BEFORE
 *   t (container - top letter = t)
 *     an ~ (word in the container)
 *     ree ~ (word in the container)
 *     rust ~ (word in the container)
 *
 *   AFTER
 *   t (node)
 *     a (container - top letter = a)
 *       n ~ (word in the container)
 *     r (container - top letter = r)
 *       ee ~ (word in the container)
 *       ust ~ (word in the container)
 *
 * The burst operation is described in detail by the paper that
 * originally described burst tries, freely available on the Internet.
 * (The HAT-trie is a derivation of a burst-trie.)
 *
 * @param htc  container to burst
 */
template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
burst(container *htc) {
    // Construct a new node.
    node *result = new node(htc->ch());
    result->set_word(htc->is_word());

    // Make a set of containers for the data in the old container and
    // add them to the new node.
    typename container::store_type::iterator it;
    for (it = htc->store.begin(); it != htc->store.end(); ++it) {
        int index = ht_get_index<alphabet_size, indexof>((*it)[0]);
        if (result->children[index] == NULL) {
            // Make a new container and position it under the new node.
            container *insertion = new container((*it)[0]);
            insertion->parent = result;
            result->children[index] = insertion;
            result->types[index] = CONTAINER_POINTER;

            // Set the new container's word field.
            insertion->set_word(((*it)[1] == '\0'));
        }
        if ((*it)[1] != '\0') {  // then the word length is > 1
            // Insert the rest of the word into the respective
            // container.
            ((container *)result->children[index])->insert((*it) + 1);
        }
    }

    // Position the new node in the trie.
    node *parent = htc->parent;
    result->parent = parent;
    int index = ht_get_index<alphabet_size, indexof>(htc->ch());
    parent->children[index] = result;
    parent->types[index] = NODE_POINTER;
    delete htc;
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
print(const node_pointer &n, const std::string &space) const {
    using namespace std;

    if (n.type == CONTAINER_POINTER) {
        container *c = (container *)n.pointer;
        if (c->ch() != '\0') {
            cout << space << c->ch();
            if (c->is_word()) {
                cout << " ~";
            }
            cout << endl;
        }

        typename container::store_type::iterator it;
        it = c->store.begin();
        for (it = c->store.begin(); it != c->store.end(); ++it) {
            cout << space + "  " << *it << " ~" << endl;
        }

    } else if (n.type == NODE_POINTER) {
        node *p = (node *)n.pointer;
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
typename hat_trie<alphabet_size, indexof>::node_pointer
hat_trie<alphabet_size, indexof>::
next_word(node_pointer n) {
    if (n.pointer == NULL) {
        return node_pointer();
    }

    node_pointer result;
    if (n.type == NODE_POINTER) {
        // Scan through n's children and go down the first available path.
        node *p = (node *) result.pointer;
        for (int i = 0; i < alphabet_size; ++i) {
            if (p->children[i]) {

            }
        }

    } else {
        // Until you can move to the right, move up.
    }

    // Find the lexicographically least word from the current position.
    return least(result);
}

// ---------
// iterators
// ---------

/**
 * Moves the iterator forward.
 *
 * @return  self-reference
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator&
hat_trie<alphabet_size, indexof>::
iterator::operator++() {

}

/**
 * Moves the iterator backward.
 *
 * @return  self-reference
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator&
hat_trie<alphabet_size, indexof>::
iterator::operator--() {

}

/**
 * Iterator dereference operator.
 *
 * @return  string this iterator points to
 */
template <int alphabet_size, int (*indexof)(char)>
std::string hat_trie<alphabet_size, indexof>::
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

