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
// TODO document which allocation scheme is better for array_hash

// INTERFACE (* for implemented, ? for see documentation)
//   set interface:
//    * iterator begin()
//    * void clear()
//    * size_t count(const key_type &) const
//    * bool empty() const
//    * iterator end()
//      pair<iterator, iterator> equal_range(const key_type &) const
//      void erase(iterator)
//      void erase(const key_type &)
//      void erase(iterator, iterator)
//    * iterator find(const key_type &) const
//      allocator_type get_allocator() csont
//    ? pair<iterator, bool> insert(const key_type &)
//    * iterator insert(iterator, const key_type &)
//    * void insert(input_iterator first, input_iterator last)
//      key_compare key_comp() const
//      iterator lower_bound(const key_type &) const
//      size_t max_size() const
//      self_reference operator=(self)
//      reverse_iterator rbegin()
//      reverse_iterator rend()
//    * size_t size() const
//    * void swap(self &)
//      iterator upper_bound(const key_type &) const
//      value_compare value_comp() const
//
//   additions:
//    * bool contains() const
//      hat_trie prefix_match(const key_type &) const

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
    // types node_base values could point to. This is stored in
    // one bit, so the only valid values are 0 and 1
    enum { NODE_POINTER = 0, CONTAINER_POINTER = 1 };

    typedef hat_trie _self;
    typedef hat_trie_node _node;
    typedef hat_trie_container _container;
    typedef hat_trie_node_base _node_base;

    // pairs node_base pointers with their type (whether they point to
    // nodes or containers)
    class _node_pointer {

      public:
        unsigned char type;
        _node_base *p;

        _node_pointer(unsigned char type = 0, _node_base *p = NULL) :
                type(type), p(p) { }

        // Conversion constructor from node * to _node_pointer type
        _node_pointer(_node *n) : type(NODE_POINTER), p(n) { }

        // comparison operators
        bool operator==(const _node_pointer &rhs)
        { return p == rhs.p; }
        bool operator!=(const _node_pointer &rhs)
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
    bool contains(const key_type &word) const;
    size_t count(const key_type &word) const;
    bool empty() const;
    key_compare key_comp() const;
    size_t size() const;
    void print(std::ostream &out = std::cout) const { _print(out, _root); }

    // modifiers
    void clear();

    bool insert(const key_type &word);
    bool insert(const char *word);
    template <class input_iterator>
    void insert(input_iterator first, const input_iterator &last);
    iterator insert(const iterator &, const key_type &word);

    void erase(const iterator &pos);
    size_t erase(const key_type &word);
    void erase(iterator first, const iterator &last);

    // iterators
    iterator begin() const;
    iterator end() const;
    iterator find(const key_type &s) const;

    // utilities
    void swap(_self &rhs);

    // comparison operators
    friend bool operator<(const _self &lhs, const _self &rhs);
    friend bool operator>(const _self &lhs, const _self &rhs);
    friend bool operator<=(const _self &lhs, const _self &rhs);
    friend bool operator>=(const _self &lhs, const _self &rhs);
    friend bool operator==(const _self &lhs, const _self &rhs);
    friend bool operator!=(const _self &lhs, const _self &rhs);

    // TODO explain all the state an iterator maintains
    //     TODO is this the best way to solve this problem?
    //      pros - done automatically so programmer won't forget
    //             simple code
    //      cons - not intuitive because assigning to a position in
    //             the trie would intuitively affect all the state
    //             in the iterator
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
        // Current position in the trie
        _node_pointer _position;

        // Internal iterator across container types
        array_hash::iterator _container_iterator;
        bool _word;

        // Caches the word as we move up and down the trie and
        // implicitly caches the path we followed as well
        key_type _cached_word;

        // Special-purpose constructor and assignment operator. If
        // an iterator is assigned to a container, it automatically
        // initializes its internal iterator to the first element
        // in that container.
        iterator(_node_pointer);
        iterator &operator=(_node_pointer);

    };

  private:
    _node *_root;  // pointer to the root of the trie
    size_t _size;  // number of distinct elements in the trie


    // containers are burst after their size crosses this threshold
    // MUST be <= 32,768
    enum { BURST_THRESHOLD = 16384 };

    void _init();

    // accessors
    _node_pointer _locate(const char *&s) const;
    void _print(std::ostream &,
                const _node_pointer &,
                const key_type & = "") const;

    static _node_pointer _next_child(_node *, size_t, key_type &);
    static _node_pointer _least_child(_node *, key_type &);
    static _node_pointer _next_word(_node_pointer, key_type &);
    static _node_pointer _least(_node_pointer, key_type &);
    static int _pop_back(key_type &);

    // modifiers
    bool _insert(_container *htc, const char *s);
    void _burst(_container *htc);

};

/**
 * Namespace-scope swap function for hat tries.
 */
void swap(hat_trie &, hat_trie &);

}  // namespace stx

#endif  // HAT_TRIE_H

