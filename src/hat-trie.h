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
//    * size_type count(const key_type &) const
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
//    * key_compare key_comp() const
//      iterator lower_bound(const key_type &) const
//      size_type max_size() const
//      self_reference operator=(self)
//      reverse_iterator rbegin()
//      reverse_iterator rend()
//    * size_type size() const
//    * void swap(self &)
//      iterator upper_bound(const key_type &) const
//    * value_compare value_comp() const
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
 * Provides a way to tune the performance characteristics of a
 * HAT-trie.
 */
class hat_trie_traits {

  public:
    hat_trie_traits() {
        burst_threshold = 16384;
    }

    /**
     * A hat_trie container is burst when its size passes this
     * threshold. Higher values use less memory, but may be slower.
     *
     * Default 16384. Must be > 0 and <= 32,768.
     */
    size_t burst_threshold;

};

/*
hat_trie public interface:
class hat_trie {

  public:
    // STL types
    typedef size_t           size_type;
    typedef std::string      key_type;
    typedef key_type         value_type;
    typedef key_type &       reference;
    typedef const key_type & const_reference;
    typedef std::less<char>  key_compare;
    typedef key_compare      value_compare;

    class iterator;
    typedef iterator const_iterator;

    // constructors and destructors
    hat_trie(hat_trie_traits traits = hat_trie_traits());
    template <class input_iterator>
    hat_trie(const input_iterator &first, const input_iterator &last);
    virtual ~hat_trie();

    // accessors
    bool contains(const key_type &word) const;
    size_t count(const key_type &word) const;
    bool empty() const;
    key_compare key_comp() const;
    size_type size() const;
    value_compare value_comp() const;
    void print(std::ostream &out = std::cout) const { _print(out, _root); }

    // modifiers
    void clear();

    bool insert(const key_type &word);
    bool insert(const char *word);
    template <class input_iterator>
    void insert(input_iterator first, const input_iterator &last);
    iterator insert(const iterator &, const key_type &word);

    void erase(const iterator &pos);
    size_type erase(const key_type &word);
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

    class iterator {

      public:
        iterator();

        iterator operator++(int);
        iterator &operator++();
        iterator operator--(int);
        iterator &operator--();

        key_type operator*() const;
        bool operator==(const iterator &rhs);
        bool operator!=(const iterator &rhs);

    };
};
*/

/**
 * Trie-based data structure for managing sorted strings.
 */
template <class T>
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
    typedef size_t           size_type;
    typedef T                key_type;
    typedef key_type         value_type;
    typedef key_type &       reference;
    typedef const key_type & const_reference;
    typedef std::less<char>  key_compare;
    typedef key_compare      value_compare;

    class iterator;
    typedef iterator const_iterator;

    /**
     * Default constructor.
     */
    hat_trie(const hat_trie_traits &traits = hat_trie_traits(),
             const array_hash_traits &ah_traits = array_hash_traits()) :
            _traits(traits), _ah_traits(ah_traits) {
        _init();
    }

    /**
     * Builds a HAT-trie from the data in [first, last).
     *
     * @param first, last  iterators specifying a range of elements
     */
    template <class input_iterator>
    hat_trie(const input_iterator &first, const input_iterator &last,
             const hat_trie_traits &traits = hat_trie_traits(),
             const array_hash_traits &ah_traits = array_hash_traits()) :
             _traits(traits), _ah_traits(ah_traits) {
        _init();
        insert(first, last);
    }

    virtual ~hat_trie() {
        delete _root;
        _root = NULL;
    }

    /**
     * Searches for a word in the trie.
     *
     * This function is an extension to the standard STL interface.
     *
     * @param word  word to search for
     * @return  true iff @a s is in the trie
     */
    bool contains(const key_type &word) const {
        // Locate s in the trie's structure.
        const char *ps = word.c_str();
        _node_pointer n = _locate(ps);

        if (n.type == CONTAINER_POINTER) {
            return ((_container *) n.p)->contains(ps);
        }
        return n.p->word();
    }

    /**
     * Determines whether this container is empty.
     *
     * @return  true iff this container has no data
     */
    bool empty() const {
        return size() == 0;
    }

    /**
     * Gets the number of distinct elements in the trie.
     *
     * @return  size of the trie
     */
    size_type size() const {
        return _size;
    }

    /**
     * Gets the traits associated with this trie.
     */
    const hat_trie_traits &traits() const {
        return _traits;
    }

    /**
     * Gets the array hash traits associated with the hash tables in
     * this trie.
     */
    const array_hash_traits &hash_traits() const {
        return _ah_traits;
    }

    // TODO
    void print(std::ostream &out = std::cout) const { _print(out, _root); }

    /**
     * Removes all the elements in the trie.
     */
    void clear() {
        delete _root;
        _init();
    }

    /**
     * Inserts a word into the trie.
     *
     * According to the standard, this function should return a
     * pair<iterator, bool> rather than just a bool. However, timing tests
     * on both versions of this function showed significant slowdown on
     * the pair-returning version -- several orders of magnitude. We believe
     * deviating from the standard in the face of such significant slowdown
     * is a worthy sacrifice for blazing fast insertion times. And besides,
     * who uses the iterator return value anyway? =)
     *
     * Note: for a more in-depth discussion of rationale, see the HTML
     * documentation.
     *
     * @param word  word to insert
     *
     * @return  true if @a word is inserted into the trie, false if @a word
     *          was already in the trie
     */
    bool insert(const key_type &word) {
        return insert(word.c_str());
    }

    /**
     * Inserts a word into the trie.
     *
     * Uses C-strings instead of C++ strings. This function is no more
     * efficient than the string version. It is provided for convenience.
     *
     * @param word  word to insert
     * @return  true if @a word is inserted into the trie, false if @a word
     *          was already in the trie
     */
    bool insert(const char *word) {
        const char *pos = word;
        _node_pointer n = _locate(pos);
        if (*pos == '\0') {
            // word was found in the trie's structure. Mark its location
            // as the end of a word.
            if (n.p->word() == false) {
                n.p->set_word(true);
                ++_size;
                return true;
            }
            return false;

        } else {
            // word was not found in the trie's structure. Either make a
            // new container for it or insert it into an already
            // existing container.
            _container *c = NULL;
            if (n.type == NODE_POINTER) {
                // Make a new container for word.
                _node *p = (_node *) n.p;
                int index = *pos;
                c = new _container(index, _ah_traits);

                // Insert the new container into the trie structure.
                c->_parent = p;
                p->_children[index] = c;
                p->_types[index] = CONTAINER_POINTER;
                ++pos;
            } else if (n.type == CONTAINER_POINTER) {
                // The container for s already exists.
                c = (_container *) n.p;
            }

            // Insert the rest of word into the container.
            return _insert(c, pos);
        }
    }

    /**
     * Inserts a word into the trie.
     *
     * In standard STL sets, this function can dramatically increase
     * performance if @a position is set correctly. This performance
     * gain is unachievable in a HAT-trie because the time required to
     * verify that @a position points to the right place is just as
     * expensive as a regular insert operation.
     *
     * @param word  word to insert
     * @return iterator to @a word in the trie
     */
    template <class input_iterator>
    void insert(input_iterator first, const input_iterator &last) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    /**
     * Inserts several words into the trie.
     *
     * @param first, last  iterators specifying a range of words to add
     *                     to the trie. All words in the range
     *                     [first, last) are added
     */
    iterator insert(const iterator &, const key_type &word) {
        insert(word);
        return find(word);
    }


    /**
     * Erases a word from the trie.
     *
     * @param pos  iterator to the word in the trie
     */
    void erase(const iterator &) {

    }

    /**
     * Erases a word from the trie.
     *
     * @param word  word to erase
     * @return  number of words erased from the trie. In a set container,
     *          either 1 if the word was removed from the trie or 0 if the
     *          word doesn't appear in the trie
     */
    size_type erase(const key_type &) {
        return 0;
    }

    /**
     * Erases several words from the trie.
     *
     * @param first, last  iterators specifying a range of words to remove
     *                     from the trie. All words in the range [first,
     *                     last) are removed
     */
    void erase(iterator first, const iterator &last) {
        while (first != last) {
            erase(first);
            ++first;
        }
    }

    /**
     * Gets an iterator to the first element in the trie.
     *
     * If there are no elements in the trie, the iterator pointing to
     * trie.end() is returned.
     *
     * @return  iterator to the first element in the trie
     */
    iterator begin() const {
        // Stop early if there are no elements in the trie.
        if (size() == 0) {
            return end();
        }

        // Incrementally construct the iterator to return. This code
        // is pretty ugly. See the doc comment for the iterator class
        // for a description of why.
        iterator result;
        result = _least(_root, result._cached_word);
        return result;
    }

    /**
     * Gets an iterator to one past the last element in the trie.
     *
     * @return iterator to one past the last element in the trie
     */
    iterator end() const {
        return iterator();
    }


    /**
     * Searches for @a s in the trie.
     *
     * @param s  word to search for
     * @return  iterator to @a s in the trie. If @a s is not in the trie,
     *          returns an iterator to one past the last element
     */
    iterator find(const key_type &s) const {
        const char *ps = s.c_str();
        _node_pointer n = _locate(ps);

        // Search for the word in the trie.
        iterator result;
        if ((n.type == CONTAINER_POINTER &&
                ((_container *) n.p)->contains(ps)) ||
                n.p->word()) {
            // The word is in the trie. Find its location and initialize the
            // return iterator to that location.
            result = n;
            result._cached_word = key_type(s.c_str(), ps);
            if (*ps != '\0') {
                result._word = false;
                result._container_iterator = ((_container *) n.p)->_store.find(ps);
            }

        } else {
            // The word wasn't found in the trie.
            result = end();
        }
        return result;
    }

    /**
     * Swaps the data in two hat_trie objects.
     *
     * This function operates in constant time because all it needs to do
     * is swap two primitive values.
     *
     * @param rhs  hat_trie object to swap data with
     */
    void swap(_self &rhs) {
        using std::swap;
        swap(_root, rhs._root);
        swap(_size, rhs._size);
        swap(_traits, rhs._traits);
    }

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


        /**
         * Moves the iterator forward.
         *
         * @return  self-reference
         */
        iterator &operator++() {
            if (_position.type == CONTAINER_POINTER) {
                // If word is set, then this container represents a word as
                // well as storing words. The first iteration into the
                // container is over the word represented by the container.
                if (_word) {
                    // Move into the actual container by setting word to false.
                    _word = false;
                } else {
                    // Move the iterator over the container's elements forward.
                    ++_container_iterator;
                }

                // If we aren't at the end of the container, stop here.
                if (_container_iterator !=
                        ((_container *) _position.p)->_store.end()) {
                    return *this;
                }
            }

            // Move to the next node in the trie.
            return (*this = hat_trie::_next_word(_position, _cached_word));
        }

        /**
         * Moves the iterator backward.
         *
         * @return  self-reference
         */
        iterator &operator--() {
            return *this;
        }

        /**
         * Moves the iterator forward.
         *
         * @return  copy of this iterator before it was moved
         */
        iterator operator++(int) {
            iterator result = *this;
            operator++();
            return result;
        }

        /**
         * Moves the iterator backward.
         *
         * @return  copy of this iterator before it was moved
         */
        iterator operator--(int) {
            iterator result = *this;
            operator--();
            return result;
        }


        /**
         * Iterator dereference operator.
         *
         * @return  string this iterator points to
         */
        key_type operator*() const {
            if (_word || _position.type == NODE_POINTER) {
                // Print the word that has been cached over the trie traversal.
                return _cached_word;

            } else if (_position.type == CONTAINER_POINTER) {
                // Pull a word from the container.
                return _cached_word + *_container_iterator;
            }

            // should never get here
            return "";
        }

        /**
         * Overloaded equivalence operator.
         *
         * @param rhs  iterator to compare against
         * @return  true iff this iterator points to the same location as
         *          @a rhs
         */
        bool operator==(const iterator &rhs) {
            // TODO does iterator comparison need to be on more than
            // just pointer?
            return _position == rhs._position;
        }

        /**
         * Overloaded not-equivalence operator.
         *
         * @param rhs  iterator to compare against
         * @return  true iff this iterator is not equal to @a rhs
         */
        bool operator!=(const iterator &rhs) {
            return !operator==(rhs);
        }

      private:
        // Current position in the trie
        _node_pointer _position;

        // Internal iterator across container types
        typename array_hash<key_type>::iterator _container_iterator;
        bool _word;

        // Caches the word as we move up and down the trie and
        // implicitly caches the path we followed as well
        key_type _cached_word;

        /**
         * Special-purpose conversion constructor.
         *
         * If an iterator is constructed from a container pointer,
         * this function ensures that the iterator's internal iterator
         * across the elements in the container is properly initialized.
         */
        iterator(_node_pointer n) {
            operator=(n);
        }

        /**
         * Special-purpose assignment operator.
         *
         * If an iterator is assigned to a container pointer, this
         * function ensures that the iterator's internal iterator across
         * the elements in the container is properly initialized.
         */
        iterator &operator=(_node_pointer n) {
            this->_position = n;
            if (_position.type == CONTAINER_POINTER) {
                _container_iterator =
                        ((_container *) _position.p)->_store.begin();
                _word = _position.p->word();
            }
            return *this;
        }

    };

  private:
    hat_trie_traits _traits;
    array_hash_traits _ah_traits;
    _node *_root;  // pointer to the root of the trie
    size_type _size;  // number of distinct elements in the trie

    void _print(std::ostream &,
                const _node_pointer &,
                const key_type & = "") const;

    /**
     * Initializes all the fields in a hat_trie as if it had just been
     * created.
     */
    void _init() {
        _size = 0;
        _root = new _node();
    }

    /**
     * Locates the position @a s should be in the trie.
     *
     * @param s  string to search for. After this function completes, if
     *           <code>*s = '\0'</code>, @a s is in the trie part of this
     *           data structure. If not, @a s is either completed in a
     *           container or is not in the trie at all.
     * @return  a _node_pointer to the location of @a s in the trie
     */
    _node_pointer _locate(const char *&s) const {
        _node *p = _root;
        _node_base *v = NULL;
        while (*s) {
            int index = *s;
            v = p->_children[index];
            if (v) {
                ++s;
                if (p->_types[index] == NODE_POINTER) {
                    // Keep moving down the trie structure.
                    p = (_node *) v;
                } else if (p->_types[index] == CONTAINER_POINTER) {
                    // s should appear in the container v
                    return _node_pointer(CONTAINER_POINTER, v);
                }
            } else {
                // s should appear underneath this node
                return _node_pointer(NODE_POINTER, p);
            }
        }

        // If we get here, no container was found that could have held
        // s, meaning node n represents s in the trie.
        return _node_pointer(NODE_POINTER, p);
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
    bool _insert(_container *htc, const char *s) {
        // Try to insert s into the container.
        if (htc->insert(s)) {
            ++_size;
            if (htc->size() > _traits.burst_threshold) {
                // The container has too many strings in it; burst the
                // container into a node.
                _burst(htc);
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
     *       ust ~ (word in the container)
     *       ee ~ (word in the container)
     *
     * The burst operation is described in detail by the paper that
     * originally described burst tries, freely available on the Internet.
     * (The HAT-trie is a derivation of a burst-trie.)
     *
     * @param htc  container to burst
     */
    void _burst(_container *htc) {
        // Construct a new node.
        _node *result = new _node(htc->ch());
        result->set_word(htc->word());

        // Make a set of containers for the data in the old container and
        // add them to the new node.
        typename array_hash<key_type>::iterator it;
        for (it = htc->_store.begin(); it != htc->_store.end(); ++it) {
            int index = (*it)[0];

            // Do we need to make a new container?
            if (result->_children[index] == NULL) {
                // Make a new container and position it under the new node.
                _container *insertion = new _container((*it)[0], _ah_traits);
                insertion->_parent = result;
                result->_children[index] = insertion;
                result->_types[index] = CONTAINER_POINTER;

                // Set the new container's word field.
                insertion->set_word(((*it)[1] == '\0'));
            }

            // Insert the rest of the word into a container.
            if ((*it)[1] != '\0') {
                // Insert the rest of the word into the right container.
                ((_container *) result->_children[index])->insert((*it) + 1);
            } else {
                // Mark this container as a word.
                ((_container *) result->_children[index])->set_word(true);
            }
        }

        // Position the new node in the trie.
        _node *p = htc->_parent;
        result->_parent = p;
        int index = htc->ch();
        p->_children[index] = result;
        p->_types[index] = NODE_POINTER;
        delete htc;
    }

    /**
     * Finds the next child under a node.
     *
     * @param p  parent node to search under
     * @param pos  starting position in the children array
     * @param word  cached word in the trie traversal
     * @return  a pointer to the next child under this node starting from
     *          @a pos, or NULL if this node has no children
     */
    static _node_pointer _next_child(_node *p, size_type pos, key_type &word) {
        _node_pointer result;

        // Search for the next child under this node starting at pos.
        for (int i = pos; i < HT_ALPHABET_SIZE && result.p == NULL; ++i) {
            if (p->_children[i]) {
                // Found a child.
                result.p = p->_children[i];
                result.type = p->_types[i];

                // Add this motion to the word.
                word += result.p->ch();
            }
        }
        return result;
    }

    /**
     * Finds the next node that marks a word.
     *
     * This node may be either a node or a container (that is itself a word
     * or has a word in it).
     *
     * @param n
     * @param word  cached word in the trie traversal
     * @return  a pointer to the next node in the trie that marks a word
     */
    static _node_pointer _next_word(_node_pointer n, key_type &word) {
        // Stop early if we get a NULL pointer.
        if (n.p == NULL) { return _node_pointer(); }

        _node_pointer result;
        if (n.type == NODE_POINTER) {
            // Move to the leftmost child under this node.
            result = _next_child((_node *) n.p, 0, word);
        }

        if (result.p == NULL) {
            // This node has no children. Move up in the trie until
            // we can move right.
            _node_pointer next;
            int pos;
            while (n.p->_parent && next.p == NULL) {
                // Looks like we can't move to the right. Move up a level
                // in the trie and try again.
                pos = _pop_back(word) + 1;
                next = _next_child(n.p->_parent, pos, word);
                n = n.p->_parent;
            }
            result = next;
        }

        // Return the lexicographically least node underneath this one.
        return _least(result, word);
    }

    /**
     * Finds the lexicographically least node starting from @a n.
     *
     * @param n     current position in the trie
     * @param word  cached word in the trie traversal
     * @return  lexicographically least node from @a n. This function
     *          may return @a n itself
     */
    static _node_pointer _least(_node_pointer n, key_type &word) {
        while (n.p && n.p->word() == false && n.type == NODE_POINTER) {
            // Find the leftmost child of this node and move in
            // that direction.
            n = _next_child((_node *) n.p, 0, word);
        }
        return n;
    }

    /**
     * Removes a path record (i.e. a character) from the back of a word.
     *
     * This function assumes @a word is populated. The string class
     * will throw an exception if this precondition is not met.
     *
     * @param word  cached word in the trie traversal
     * @return  integer that was formerly the most recent path taken
     */
    static int _pop_back(key_type &word) {
        int result = word[word.size() - 1];
        word.erase(word.size() - 1);
        return result;
    }

  public:
    // comparison operators
    friend bool operator<(const _self &lhs, const _self &rhs);
    friend bool operator>(const _self &lhs, const _self &rhs);
    friend bool operator<=(const _self &lhs, const _self &rhs);
    friend bool operator>=(const _self &lhs, const _self &rhs);
    friend bool operator==(const _self &lhs, const _self &rhs);
    friend bool operator!=(const _self &lhs, const _self &rhs);

};

/**
 * Recursively prints the contents of the trie.
 */
template <class T>
void
hat_trie<T>::_print(
        std::ostream &out,
        const _node_pointer &n,
        const key_type &space) const {
    if (n.type == CONTAINER_POINTER) {
        _container *c = (_container *) n.p;
        if (c->ch() != '\0') {
            out << space << c->ch();
            if (c->word()) {
                out << " ~";
            }
            out << std::endl;
        }

        typename array_hash<key_type>::iterator it;
        it = c->_store.begin();
        for (it = c->_store.begin(); it != c->_store.end(); ++it) {
            out << space + "  " << *it << " ~" << std::endl;
        }

    } else if (n.type == NODE_POINTER) {
        _node *p = (_node *) n.p;
        if (p->ch() != '\0') {
            out << space << p->ch();
            if (p->word()) {
                out << " ~";
            }
            out << std::endl;
        }
        for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
            if (p->_children[i]) {
                _print(out, _node_pointer(p->_types[i],
                       p->_children[i]), space + "  ");
            }
        }
    }
}

/**
 * Namespace-scope swap function for hat tries.
 */
//template <class T>
//void swap(hat_trie<T> &lhs, hat_trie<T> &rhs) {
    //lhs.swap(rhs);
//}

// --------------------
// COMPARISON OPERATORS
// --------------------

template <class T>
bool
operator<(const stx::hat_trie<T> &lhs,
          const stx::hat_trie<T> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
}
template <class T>
bool
operator==(const stx::hat_trie<T> &lhs,
           const stx::hat_trie<T> &rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}
template <class T>
bool
operator>(const stx::hat_trie<T> &lhs,
          const stx::hat_trie<T> &rhs) {
    return rhs < lhs;
}
template <class T>
bool
operator<=(const stx::hat_trie<T> &lhs,
           const stx::hat_trie<T> &rhs) {
    return !(rhs < lhs);
}
template <class T>
bool
operator>=(const stx::hat_trie<T> &lhs,
           const stx::hat_trie<T> &rhs) {
    return !(lhs < rhs);
}
template <class T>
bool
operator!=(const stx::hat_trie<T> &lhs,
           const stx::hat_trie<T> &rhs) {
    return !(lhs == rhs);
}

}  // namespace stx

namespace std {

/**
 * Template overload of std::swap for hat_tries.
 *
 * According to the standard, this technically isn't allowed, but there
 * is no way to solve this problem without breaking standard. Boost
 * libraries do it (see smart pointer library).
 */
template <class T>
void
swap(stx::hat_trie<T> &lhs, stx::hat_trie<T> &rhs) {
    lhs.swap(rhs);
}

}  // namespace std

#endif  // HAT_TRIE_H

