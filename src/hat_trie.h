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
//    * void erase(iterator)
//    * void erase(const key_type &)
//    * void erase(iterator, iterator)
//    * iterator find(const key_type &) const
//      allocator_type get_allocator() csont
//    ? pair<iterator, bool> insert(const value_type &)
//    * iterator insert(iterator, const value_type &)
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
//    * bool exists() const
//      hat_trie prefix_match(const key_type &) const

#ifndef HAT_TRIE_H
#define HAT_TRIE_H

#include <iostream>  // for std::ostream
#include <string>
#include <bitset>

#include "array_hash.h"

/// TODO
#include <iostream>
using namespace std;

namespace stx {

/// number of distinct characters a hat trie can store
const int HT_ALPHABET_SIZE = 128;

typedef array_hash<std::string> bucket;

/**
 * Provides a way to tune the performance characteristics of a HAT-trie.
 *
 * \subsection Usage
 * \code
 * hat_trie_traits traits;
 * traits.burst_threshold = 8192;
 * hat_set<string> rawr(traits);
 * rawr.insert(...);
 * ...
 * \endcode
 */
class hat_trie_traits {

  public:
    hat_trie_traits(size_t burst_threshold = 16384) {
        this->burst_threshold = burst_threshold;
    }

    /**
     * A hat_trie container is burst when its size passes this
     * threshold. Higher values use less memory, but may be slower.
     *
     * Set this value to 0 to turn this data structure into a
     * retrieval tree and leave out array hashes completely.
     *
     * Default 16384. Must be >= 0 and <= 32,768.
     */
    size_t burst_threshold;

};

/// Gets a reference to the string in the parameter
template <class T> const std::string &ref(const T &t);

const std::string &ref(const std::string &s) {
    return s;
}

template <class T>
const std::string &ref(const std::pair<std::string, T> &p) {
    return p.first;
}

// forward declarations
class htnode;
struct ahnode;

union child_ptr {
    ahnode *bucket;
    htnode *node;
};

/// Stores information required by each hat trie node
class htnode {
    friend class hat_trie<std::string>;

  public:
    htnode(char ch = '\0') {
        memset(_children, NULL, sizeof(child_ptr) * HT_ALPHABET_SIZE);
    }

    /// Getter for the word field
    bool word() const { return _types[HT_ALPHABET_SIZE]; }

    /// Setter for the word field
    void set_word(bool b) { _types[HT_ALPHABET_SIZE] = b; }

    char ch;
    htnode *parent;

  private:
    std::bitset<HT_ALPHABET_SIZE + 1> _types;  // +1 is an end of word flag
    child_ptr _children[HT_ALPHABET_SIZE];  // pointers to children
};

struct ahnode {
    bucket *table;
    char ch;
    bool word;
    htnode *parent;
};

/// valid values for an htnode_ptr
enum { NODE_POINTER = 0, BUCKET_POINTER = 1 };

struct htnode_ptr {
    child_ptr ptr;  // pointer to a node in the trie
    uint8_t type;   // type of the pointer

    htnode_ptr() { }
    htnode_ptr(child_ptr ptr, uint8_t type) : ptr(ptr), type(type) { }
    htnode_ptr(htnode *node) {
        ptr.node = node;
        type = NODE_POINTER;
    }
    htnode_ptr(ahnode *bucket) {
        ptr.bucket = bucket;
        type = BUCKET_POINTER;
    }

    bool word() {
        return type == NODE_POINTER ? ptr.node->word() : ptr.bucket->word;
    }

    void set_word(bool value) {
        if (type == NODE_POINTER) {
            ptr.node->set_word(value);
        } else if (type == BUCKET_POINTER) {
            ptr.bucket->word = value;
        }
    }

    char ch() {
        return type == NODE_POINTER ? ptr.node->ch : ptr.bucket->ch;
    }

    htnode *parent() {
        return type == NODE_POINTER ? ptr.node->parent : ptr.bucket->parent;
    }
};

/**
 * Trie-based data structure for managing sorted strings.
 */
template <>
class hat_trie<std::string> {

  private:
    typedef hat_trie _self;

  public:
    // STL types
    typedef size_t           size_type;
    typedef std::string      key_type;
    typedef std::string      value_type;
    typedef std::less<char>  key_compare;

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
     * Array hash traits constructor.
     */
    hat_trie(const array_hash_traits &ah_traits) :
            _ah_traits(ah_traits) {
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
    bool exists(const key_type &word) const {
        // Locate s in the trie's structure.
        const char *ps = word.c_str();
        htnode_ptr n = _locate(ps);

        if (*ps == '\0') {
            // The string was found in the trie's structure
            return n.word();
        }
        if (n.type == BUCKET_POINTER) {
            // Determine whether the remainder of the string is inside
            // a container or not
            return n.ptr.bucket->table->exists(ps);
        }
        return false;
    }

    /**
     * Counts the number of times a word appears in the trie.
     *
     * In distinct containers, this number will either be 1 or 0.
     *
     * @param word  word to search for
     * @return  number of times @a word appears in the trie
     */
    size_type count(const key_type &word) const {
        return exists(word) ? 1 : 0;
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

    /**
     * Prints the hierarchical structure of the trie.
     *
     * The output is indented to indicate trie depth. Words are marked
     * by a ~, and containers are marked by a *. For example, a trie with
     * a @a burst_threshold of 2 with the words the, their, there, they're,
     * train, trust, truth, bear, and breath would produce this output:
     *
     *   b *
     *     reath ~
     *     ear ~
     *   t
     *     h
     *      e ~
     *        r *
     *          e ~
     *        y *
     *          `re ~
     *        i *
     *          r ~
     *     r
     *       a *
     *         in ~
     *       u *
     *         st ~
     *         th ~
     *
     * (This isn't exactly right because of the particular bursting
     * algorithm this implementation uses, but it is a good example.)
     *
     * @param out  output stream to print to. cout by default
     */
    void print(std::ostream &out = std::cout) const {
        _print(out, _root);
    }

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
     * is a worthy sacrifice for blazing fast insertion times.
     *
     * @param word  word to insert
     *
     * @return  true if @a word is inserted into the trie, false if @a word
     *          was already in the trie
     */
    bool insert(const key_type &key) {
        const std::string &word = ref(key);
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
        //cout << "inserting " << word << endl;
        const char *pos = word;
        htnode_ptr n = _locate(pos);
        if (*pos == '\0') {
            //cout << "  found" << endl;
            // word was found in the trie's structure. Mark its location
            // as the end of a word.
            if (n.word() == false) {
                n.set_word(true);
                ++_size;
                return true;
            }

            // word was already in the trie
            return false;

        } else {
            //cout << "  not found" << endl;
            // word was not found in the trie's structure. Either make a
            // new bucket for it or insert it into an already
            // existing bucket
            ahnode *at = NULL;
            if (n.type == NODE_POINTER) {
                //cout << "    making a new one" << endl;
                // Make a new bucket for word
                htnode *p = n.ptr.node;
                int index = *pos;

                at = new ahnode();
                at->table = new bucket(_ah_traits);
                //cout << "      made a new one at " << at << endl;
                at->ch = index;
                at->word = false;

                // Insert the new bucket into the trie's structure
                //cout << "      size: " << at->size() << endl;
                //cout << "      parent: " << at->parent << endl;
                at->parent = p;
                //cout << "      size: " << (void *)at->size() << endl;
                //cout << "      parent: " << at->parent << " " << p << endl;
                p->_children[index].bucket = at;
                p->_types[index] = BUCKET_POINTER;
                ++pos;
            } else if (n.type == BUCKET_POINTER) {
                //cout << "    found one already" << endl;
                // The container for s already exists.
                at = n.ptr.bucket;
            }

            // Insert the rest of word into the container.
            return _insert(at, pos);
        }
    }

    /**
     * Inserts several words into the trie.
     *
     * @param first, last  iterators specifying a range of words to add
     *                     to the trie. All words in the range
     *                     [first, last) are added
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
     * In standard STL sets, this function can dramatically increase
     * performance if the iterator is set correctly. This performance
     * gain is unachievable in a HAT-trie because the time required to
     * verify that the iterator points to the right place is just as
     * expensive as a regular insert operation. This function is
     * provided to meet the standard, but that's about it.
     *
     * @param word  word to insert
     * @return  iterator to @a word in the trie
     */
    iterator insert(const iterator &, const key_type &key) {
        const std::string &word = ref(key);
        insert(word);
        return find(word);
    }

    /**
     * Erases a word from the trie.
     *
     * @param pos  iterator to the word in the trie. Must point to a word
     *             that exists somewhere in the trie.
     */
    void erase(const iterator &pos) {
        htnode *current = NULL;
        if (pos._position.type == BUCKET_POINTER) {
            ahnode *b = pos._position.ptr.bucket;
            if (pos._word) {
                b->word = false;
            } else {
                b->table->erase(pos._container_iterator);
            }

            if (b->table->size() == 0 && b->word == false) {
                current = b->parent;
                delete b->table;
                delete b;

                // Mark the container's slot in its parent's _children
                // array as NULL.
                for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
                    if (current->_children[i].bucket == b) {
                        current->_children[i].bucket = NULL;
                        break;
                    }
                }
            }

        } else {
            current = pos._position.ptr.node;
            current->set_word(false);
        }
        --_size;

        _erase_empty_nodes(current);
    }

    /**
     * Erases a word from the trie.
     *
     * @param word  word to erase
     * @return  number of words erased from the trie. In a distinct
     *          container, either 1 if the word was removed from the
     *          trie or 0 if the word doesn't appear in the trie
     */
    size_type erase(const key_type &key) {
        const char *ps = ref(key).c_str();
        htnode_ptr n = _locate(ps);
        htnode *current = NULL;
        int result = 0;

        if (n.type == BUCKET_POINTER) {
            // The word is either in a container or is represented by the
            // container itself.
            ahnode *b = n.ptr.bucket;
            result = b->table->erase(ps);
            if (result > 0 && b->table->size() == 0 && b->word == false) {
                // Erase the container.
                current = b->parent;
                delete b->table;
                delete b;

                // Mark the container's slot in its parent's _children
                // array as NULL.
                for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
                    if (current->_children[i].bucket == b) {
                        current->_children[i].bucket = NULL;
                        break;
                    }
                }
            }

        } else {
            // The word is represented by a node in the trie. Set the word
            // field on the node to false.
            current = n.ptr.node;
            current->set_word(false);
            result = 1;
        }

        _erase_empty_nodes(current);
        _size -= result;
        return result;
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
    iterator find(const key_type &key) const {
        const std::string &word = ref(key);
        const char *ps = word.c_str();
        htnode_ptr n = _locate(ps);

        iterator result;
        if (*ps == '\0') {
            // The word is in the trie at the node returned by _locate
            if (n.word()) {
                result = n;
                result._cached_word = std::string(word.c_str());
            } else {
                // The word is not a word in the trie
                result = end();
            }
        } else {
            if (n.type == BUCKET_POINTER) {
                // The word could be in this container
                ahnode *b = n.ptr.bucket;
                bucket::iterator it = b->table->find(ps);
                if (it != b->table->end()) {
                    // The word is in the trie
                    result._position = n;
                    result._word = false;
                    result._cached_word = std::string(word.c_str(), ps);
                    result._container_iterator = it;
                } else {
                    // The word is not in the trie
                    result = end();
                }
            } else {
                // The word is not in the trie
                result = end();
            }
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
        /**
         * Default constructor.
         */
        iterator() { }

        /**
         * Moves the iterator forward.
         *
         * @return  self-reference
         */
        iterator &operator++() {
            if (_position.type == BUCKET_POINTER) {
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
                if (_container_iterator != _position.ptr.bucket->table->end()) {
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

            } else if (_position.type == BUCKET_POINTER) {
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
            return _position.ptr.bucket == rhs._position.ptr.bucket;
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
        htnode_ptr _position;

        // Internal iterator across container types
        typename bucket::iterator _container_iterator;
        bool _word;

        // Caches the word as we move up and down the trie and
        // implicitly caches the path we followed as well
        std::string _cached_word;

        /**
         * Special-purpose conversion constructor.
         *
         * If an iterator is constructed from a container pointer,
         * this function ensures that the iterator's internal iterator
         * across the elements in the container is properly initialized.
         */
        iterator(htnode_ptr n) {
            operator=(n);
        }

        /**
         * Special-purpose assignment operator.
         *
         * If an iterator is assigned to a container pointer, this
         * function ensures that the iterator's internal iterator across
         * the elements in the container is properly initialized.
         */
        iterator &operator=(htnode_ptr n) {
            this->_position = n;
            if (_position.type == BUCKET_POINTER) {
                _container_iterator = _position.ptr.bucket->table->begin();
                _word = _position.ptr.bucket->word;
            }
            return *this;
        }

    };

  private:
    hat_trie_traits _traits;
    array_hash_traits _ah_traits;
    htnode *_root;  // pointer to the root of the trie
    size_type _size;  // number of distinct elements in the trie

    /**
     * Recursively prints the contents of the trie.
     *
     * See the doc comment on print()
     *
     * @param out  output stream to print to
     * @param n  node to start recursing from
     * @param space  spaces to indent this level of the trie
     */
    void _print(std::ostream &out,
                const htnode_ptr &n,
                const std::string &space = "") const {
        if (n.type == BUCKET_POINTER) {
            ahnode *b = n.ptr.bucket;
            if (b->ch != '\0') {
                out << space << b->ch << " *";
                if (b->word) {
                    out << "~";
                }
                out << std::endl;
            }

            typename bucket::iterator it;
            it = b->table->begin();
            for (it = b->table->begin(); it != b->table->end(); ++it) {
                out << space + "  " << *it << " ~" << std::endl;
            }

        } else if (n.type == NODE_POINTER) {
            htnode *p = n.ptr.node;
            out << space << p->ch;
            if (p->word()) {
                out << " ~";
            }
            out << std::endl;
            for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
                if (p->_children[i].bucket) {
                    _print(out, htnode_ptr(p->_children[i], p->_types[i]),
                           space + "  ");
                }
            }
        }
    }

    /**
     * Initializes all the fields in a hat_trie as if it had just been
     * created.
     */
    void _init() {
        _size = 0;
        _root = new htnode();
    }

    /**
     * Locates the position @a s should be in the trie.
     *
     * @param s  string to search for. After this function completes, if
     *           <code>*s = '\0'</code>, @a s is in the trie part of this
     *           data structure. If not, @a s is either completed in a
     *           container or is not in the trie at all.
     * @return  a htnode_ptr to the location where @a s should appear
     *          in the trie
     */
    htnode_ptr _locate(const char *&s) const {
        htnode *p = _root;
        child_ptr v;
        while (*s) {
            int index = *s;
            v = p->_children[index];
            if (v.bucket) {
                ++s;
                if (p->_types[index] == NODE_POINTER) {
                    // Keep moving down the trie structure.
                    p = v.node;
                } else if (p->_types[index] == BUCKET_POINTER) {
                    // s should appear in the container v
                    return htnode_ptr(v, BUCKET_POINTER);
                }
            } else {
                // s should appear underneath this node
                return htnode_ptr(p);
            }
        }

        // If we get here, no container was found that could have held
        // s, meaning node n represents s in the trie.
        return htnode_ptr(p);
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
    bool _insert(ahnode *htc, const char *s) {
        //cout << "top of _insert. size: " << htc->size() << endl;
        // Try to insert s into the container.
        bool result;
        if (*s == '\0') {
            //cout << "  no more" << endl;
            result = !htc->word;
            htc->word = true;
        } else {
            //cout << "  more" << endl;
            result = htc->table->insert(s);
        }

        if (result) {
            ++_size;
            if (_traits.burst_threshold > 0 &&
                    htc->table->size() > _traits.burst_threshold) {
                // burst the bucket into nodes
                //cout << "BURSTING: " << htc->size() << " > " << _traits.burst_threshold << endl;
                _burst(htc);
            }
            return true;
        }
        return false;
    }

    /**
     * Starting from @a current, erases all the empty nodes up the trie.
     *
     * A node is empty if it doesn't represent a word and it has no
     * children.
     *
     * @param current  node to start from
     */
    void _erase_empty_nodes(htnode *current) {
        while (current && current != _root && current->word() == false) {
            // Erase all the nodes that aren't words and don't
            // have any children above the erased node or container.
            // Start by determining whether the current node has any
            // children.
            bool children = false;
            for (int i = 0; i < HT_ALPHABET_SIZE && !children; ++i) {
                children |= (bool)current->_children[i].bucket;
            }

            // If the current node doesn't have any children and isn't a
            // word, delete it.
            if (children == false) {
                htnode *tmp = current;
                current = current->parent;
                delete tmp;

                // Mark the slot in current's parent's _children array
                // as NULL.
                for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
                    if (current->_children[i].node == tmp) {
                        current->_children[i].node = NULL;
                        break;
                    }
                }
            } else {
                // Stop the while loop.
                current = NULL;
            }
        }
    }

    /**
     * Bursts a container into a node with containers underneath it.
     *
     * If this container contains the words tan, tree, and trust, it
     * will be split into a node with two containers underneath it. The
     * structure will look like this:
     *
     *   BEFORE
     *   t *
     *     an ~
     *     ree ~
     *     rust ~
     *
     *   AFTER
     *   t
     *     a *
     *       n ~
     *     r *
     *       ust ~
     *       ee ~
     *
     * Note: see the doc comment on print() if this notation doesn't make
     * sense.
     *
     * The burst operation is described in detail by the paper that
     * originally described burst tries, freely available on the Internet.
     * (The HAT-trie is a derivation of a burst-trie.)
     *
     * @param htc  container to burst
     */
    void _burst(ahnode *htc) {
        // Construct a new node.
        htnode *result = new htnode(htc->ch);
        result->set_word(htc->word);

        // Make a set of containers for the data in the old container and
        // add them to the new node.
        typename bucket::iterator it;
        for (it = htc->table->begin(); it != htc->table->end(); ++it) {
            int index = (*it)[0];

            // Do we need to make a new container?
            if (result->_children[index].bucket == NULL) {
                // Make a new container and position it under the new node.
                ahnode *insertion = new ahnode();
                insertion->table = new bucket(_ah_traits);
                insertion->ch = (*it)[0];
                insertion->parent = result;
                result->_children[index].bucket = insertion;
                result->_types[index] = BUCKET_POINTER;

                // Set the new container's word field.
                insertion->word = (*it)[1] == '\0';
            }

            // Insert the rest of the word into a container.
            result->_children[index].bucket->table->insert(*it + 1);  /// TODO lolwut?
            //((_container *)result->_children[index])->insert(*(it + 1));
        }

        // Position the new node in the trie.
        htnode *p = htc->parent;
        result->parent = p;
        int index = htc->ch;
        p->_children[index].node = result;
        p->_types[index] = NODE_POINTER;
        delete htc->table;
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
    static htnode_ptr _next_child(htnode *p, size_type pos, key_type &word) {
        htnode_ptr result;

        // Search for the next child under this node starting at pos.
        for (int i = pos; i < HT_ALPHABET_SIZE && result.ptr.node == NULL; ++i) {
            if (p->_children[i].node) {
                // Move to the child we just found.
                result.ptr.node = p->_children[i].node;
                result.type = p->_types[i];

                // Add this motion to the word.
                word += result.ch();
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
     * @param n  node to start from
     * @param word  cached word in the trie traversal
     * @return  a pointer to the next node in the trie that marks a word
     */
    static htnode_ptr _next_word(htnode_ptr n, key_type &word) {
        // Stop early if we get a NULL pointer.
        if (n.ptr.node == NULL) { return htnode_ptr(); }

        htnode_ptr result;
        if (n.type == NODE_POINTER) {
            // Move to the leftmost child under this node.
            result = _next_child(n.ptr.node, 0, word);
        }

        if (result.ptr.node == NULL) {
            // This node has no children. Move up in the trie until
            // we can move right.
            htnode_ptr next;
            int pos;
            htnode *parent = n.parent();
            while (parent && next.ptr.node == NULL) {
                // Looks like we can't move to the right. Move up a level
                // in the trie and try again.
                pos = _pop_back(word) + 1;
                next = _next_child(parent, pos, word);
                n = parent;
                parent = n.ptr.node->parent;
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
    static htnode_ptr _least(htnode_ptr n, key_type &word) {
        while (n.ptr.node && n.word() == false && n.type == NODE_POINTER) {
            // Find the leftmost child of this node and move in
            // that direction.
            n = _next_child(n.ptr.node, 0, word);
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
    template <class F>
    friend bool operator<(const hat_trie<F> &lhs, const hat_trie<F> &rhs);
    template <class F>
    friend bool operator>(const hat_trie<F> &lhs, const hat_trie<F> &rhs);
    template <class F>
    friend bool operator<=(const hat_trie<F> &lhs, const hat_trie<F> &rhs);
    template <class F>
    friend bool operator>=(const hat_trie<F> &lhs, const hat_trie<F> &rhs);
    template <class F>
    friend bool operator==(const hat_trie<F> &lhs, const hat_trie<F> &rhs);
    template <class F>
    friend bool operator!=(const hat_trie<F> &lhs, const hat_trie<F> &rhs);

};

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

#endif  // HAT_TRIE_H

