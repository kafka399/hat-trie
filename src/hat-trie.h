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

/**
 * \mainpage
 *
 * This project is intended to be a fully operational, standards compliant
 * HAT-trie. It will eventually mirror the STL set interface, but for now,
 * it is still VERY much a work in progress.
 *
 * \section Working
 * Here is a list of all the major operations that are working:
 *
 * \li \c insert(string)
 * \li \c contains(string)
 * \li \c find(string)
 * \li forward iteration and iterator dereferencing
 *
 * \section Usage
 *
 * \subsection Installation
 * Copy all the headers into a directory in your PATH and include hat-trie.h
 * in your project. It is designed to be a drop-in replacement for an STL set,
 * with a few changes.
 *
 * Note: some of the headers require \c inttypes.h, which isn't available by
 * default on Windows platforms. You can find a compatible version of the
 * header on Google.
 *
 * \subsection Declaration
 *
 * \li size of the alphabet (defined as the set of possible characters)
 * \li \c indexof(char) function that indexes characters in the alphabet
 *
 * \c indexof()should return an integer in [0, \c HT_ALPHABET_SIZE) that is
 * unique for each character in the alphabet.
 *
 * Here is an example \c indexof() function that indexes alphanumeric
 * characters:
 *
 * \code
 * int alphanumeric_index(char ch) {
 *     if (ch >= '0' && ch <= '9') {
 *         return ch - '0';
 *     }
 *     return ch - 'a' + 10;
 * }
 * \endcode
 *
 * In this case, the first 10 index values are the characters 0-9, and the
 * next 26 index values are the characters a-z. Note that any value outside
 * the range [0, \c HT_ALPHABET_SIZE) indicates an invalid character. If a
 * hat_trie finds an invalid character, an \c unindexed_character exception
 * is thrown.
 *
 * To declare a hat_trie that supports this alphabet:
 *
 * \code
 * hat_trie ht;
 * ht.insert(...);
 * \endcode
 */

// TODO
// what if bursting gives you a container that still has more elements than
// BURST_THRESHOLD? I think bursting this one lazily (on the next time you
// insert into this container) is acceptable. No container will have more
// values in it than BURST_THRESHOLD + 1.
//   NO! it accumulates!
// TODO documentation that limits string length to 65k characters
// TODO visual studio compatibility
// TODO code structure in hat-trie-node.h
// TODO decide which allocation scheme is better for array_hash

#ifndef HAT_TRIE_H
#define HAT_TRIE_H

#include <iostream>  // for std::ostream
#include <string>

#include "array-hash.h"
#include "hat-trie-node.h"

namespace stx {

/**
 * Trie-based data structure for managing sorted strings.
 */
class hat_trie {

  private:
    typedef hat_trie self;
    typedef hat_trie_node node;
    typedef hat_trie_container container;
    typedef hat_trie_node_base node_base;

    // pairs node_base pointers with their type (whether they point to
    // nodes or containers)
    class node_pointer {

      public:
        unsigned char type;
        node_base *pointer;

        node_pointer(unsigned char type = 0, node_base *pointer = NULL) :
                type(type), pointer(pointer) { }

        // Conversion constructor from node * to node_pointer type
        node_pointer(node *n) : type(NODE_POINTER), pointer(n) { }

        // comparison operators
        bool operator==(const node_pointer &rhs)
        { return pointer == rhs.pointer; }
        bool operator!=(const node_pointer &rhs)
        { return !operator==(rhs); }
    };

  public:
    // STL types
    typedef std::string     key_type;
    typedef key_type        value_type;
    typedef key_type&       reference;
    typedef const key_type& const_reference;

    class iterator;
    typedef iterator const_iterator;

    // constructors and destructors
    hat_trie();
    template <class input_iterator>
    hat_trie(const input_iterator &first, const input_iterator &last);
    virtual ~hat_trie();

    // accessors
    bool contains(const key_type &s) const;
    bool empty() const;
    size_t size() const;
    void print(std::ostream &out = std::cout) const { _print(out, _root); }

    // modifiers
    void clear();
    bool insert(const key_type &word);
    bool insert(const char *word);
    template <class input_iterator>
    void insert(input_iterator first, const input_iterator &last);
    iterator insert(const iterator &, const key_type &word);

    // iterators
    iterator begin() const;
    iterator end() const;
    iterator find(const key_type &s) const;

    // utilities
    void swap(self &rhs);

    // comparison operators
    friend bool operator<(const self &lhs, const self &rhs);
    friend bool operator>(const self &lhs, const self &rhs);
    friend bool operator<=(const self &lhs, const self &rhs);
    friend bool operator>=(const self &lhs, const self &rhs);
    friend bool operator==(const self &lhs, const self &rhs);
    friend bool operator!=(const self &lhs, const self &rhs);

    // TODO explain all the state an iterator maintains
    //     TODO is this the best way to solve this problem?
    //      pros - done automatically so programmer won't forget
    //             simple code
    //      cons - not intuitive because assigning to a position in
    //             the trie would intuitively affect all the state
    //             in the iterator
    // TODO what happens when you -- from begin() or ++ from end()?
    /**
     * Iterates over the elements in a HAT-trie.
     *
     * A HAT-trie iterator has to maintain a lot of state to determine
     * its current position. The code to construct iterators is pretty
     * ugly, but they have to be constructed incrementally because of
     * the large amount of state they maintain.
     */
    class iterator : public std::iterator<std::bidirectional_iterator_tag,
                                          const key_type> {
        friend class hat_trie;

      public:
        iterator() { }

        iterator operator++(int);
        iterator &operator++();
        iterator operator--(int);
        iterator &operator--();

        key_type operator*() const;
        bool operator==(const iterator &rhs);
        bool operator!=(const iterator &rhs);

      private:
        // Current location in the trie
        node_pointer n;

        // Internal iterator across container types
        ht_array_hash::iterator container_iterator;
        bool word;

        // Caches the word as we move up and down the trie and
        // implicitly caches the path we followed as well
        key_type cached_word;

        // Special-purpose constructor and assignment operator. If
        // an iterator is assigned to a container, it automatically
        // initializes its internal iterator to the first element
        // in that container.
        iterator(node_pointer);
        iterator &operator=(node_pointer);

    };

  private:
    node *_root;  // pointer to the root of the trie
    size_t _size;  // number of distinct elements in the trie

    // types node_base values could point to. This is stored in
    // one bit, so the only valid values are 0 and 1
    enum { NODE_POINTER = 0, CONTAINER_POINTER = 1 };

    // containers are burst after their size crosses this threshold
    // MUST be <= 32,768
    enum { BURST_THRESHOLD = 16384 };

    void _init();

    // accessors
    node_pointer _locate(const char *&s) const;
    void _print(std::ostream &,
                const node_pointer &,
                const key_type & = "") const;

    static node_pointer _next_child(node *, size_t, key_type &);
    static node_pointer _least_child(node *, key_type &);
    static node_pointer _next_word(node_pointer, key_type &);
    static node_pointer _least(node_pointer, key_type &);
    static int _pop_back(key_type &);

    // modifiers
    bool _insert(container *htc, const char *s);
    void _burst(container *htc);

};

/**
 * Namespace-scope swap function for hat tries.
 */
void swap(hat_trie &, hat_trie&);

}  // namespace stx

#endif  // HAT_TRIE_H

