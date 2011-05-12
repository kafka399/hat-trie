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

#include <algorithm>  // for std::lexicographical_compare and std::equal

#include "hat-trie.h"

namespace stx {

// -----------------------
// hat_trie implementation
// -----------------------

/**
 * Default constructor.
 */
hat_trie::hat_trie() {
    _init();
}

/**
 * Iterator-based constructor.
 *
 * Builds a HAT-trie from the data between the two iterators.
 *
 * @param first, last  iterators specifying a range of elements
 */
template <class input_iterator>
hat_trie::hat_trie(const input_iterator &first, const input_iterator &last) {
    _init();
    insert(first, last);
}

hat_trie::~hat_trie() {
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
bool
hat_trie::contains(const key_type &word) const {
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
bool
hat_trie::empty() const {
    return size() == 0;
}

/**
 * Gets the number of distinct elements in the trie.
 *
 * @return  size of the trie
 */
size_t
hat_trie::size() const {
    return _size;
}

/**
 * Removes all the elements in the trie.
 */
void
hat_trie::clear() {
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
bool
hat_trie::insert(const key_type &word) {
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
bool
hat_trie::insert(const char *word) {
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
            c = new _container(index);

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
hat_trie::iterator
hat_trie::insert(const iterator &, const key_type &word) {
    insert(word);
    return find(word);
}

/**
 * Inserts several words into the trie.
 *
 * @param first, last  iterators specifying a range of words to add
 *                     to the trie. All words in the range [first, last)
 *                     are added
 */
template <class input_iterator>
void
hat_trie::insert(input_iterator first, const input_iterator &last){
    while (first != last) {
        insert(*first);
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
hat_trie::iterator
hat_trie::begin() const {
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
 * Searches for @a s in the trie.
 *
 * @param s  word to search for
 * @return  iterator to @a s in the trie. If @a s is not in the trie,
 *          returns an iterator to one past the last element
 */
hat_trie::iterator
hat_trie::find(const key_type &s) const {
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
void
hat_trie::swap(_self &rhs) {
    using std::swap;
    swap(_root, rhs._root);
    swap(_size, rhs._size);
}

/**
 * Gets an iterator to one past the last element in the trie.
 *
 * @return iterator to one past the last element in the trie
 */
hat_trie::iterator
hat_trie::end() const {
    return iterator();
}

/**
 * Initializes all the fields in a hat_trie as if it had just been
 * created.
 */
void
hat_trie::_init() {
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
hat_trie::_node_pointer
hat_trie::_locate(const char *&s) const {
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
bool
hat_trie::_insert(_container *htc, const char *s) {
    // Try to insert s into the container.
    if (htc->insert(s)) {
        ++_size;
        if (htc->size() > BURST_THRESHOLD) {
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
void
hat_trie::_burst(_container *htc) {
    // Construct a new node.
    _node *result = new _node(htc->ch());
    result->set_word(htc->word());

    // Make a set of containers for the data in the old container and
    // add them to the new node.
    array_hash::iterator it;
    for (it = htc->_store.begin(); it != htc->_store.end(); ++it) {
        int index = (*it)[0];

        // Do we need to make a new container?
        if (result->_children[index] == NULL) {
            // Make a new container and position it under the new node.
            _container *insertion = new _container((*it)[0]);
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
 * Recursively prints the contents of the trie.
 */
void
hat_trie::_print(
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

        array_hash::iterator it;
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
 * Finds the next child under a node.
 *
 * @param p  parent node to search under
 * @param pos  starting position in the children array
 * @param word  cached word in the trie traversal
 * @return  a pointer to the next child under this node starting from
 *          @a pos, or NULL if this node has no children
 */
hat_trie::_node_pointer
hat_trie::_next_child(_node *p, size_t pos, key_type &word) {
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
 * This node may be either a node or a container.
 *
 * @param n
 * @param word  cached word in the trie traversal
 * @return  a pointer to the next node in the trie that marks a word
 */
hat_trie::_node_pointer
hat_trie::_next_word(_node_pointer n, key_type &word) {
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
hat_trie::_node_pointer
hat_trie::_least(_node_pointer n, key_type &word) {
    while (n.p && n.p->word() == false && n.type == NODE_POINTER) {
        // Find the leftmost child of this node and move in that direction.
        n = _next_child((_node *) n.p, 0, word);
    }
    return n;
}

/**
 * Removes a record from the back of a word.
 *
 * This function assumes @a word is populated. The string class
 * will throw an exception if this precondition is not met.
 *
 * @param word  cached word in the trie traversal
 * @return  integer that was formerly the most recent path taken
 */
int
hat_trie::_pop_back(key_type &word) {
    int result = word[word.size() - 1];
    word.erase(word.size() - 1);
    return result;
}

// ---------
// iterators
// ---------

/**
 * Moves the iterator forward.
 *
 * @return  self-reference
 */
hat_trie::iterator&
hat_trie::iterator::operator++() {
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
        if (_container_iterator != ((_container *) _position.p)->_store.end()) {
            return *this;
        }
    }

    // Move to the next node in the trie.
    return (*this = hat_trie::_next_word(_position, _cached_word));
}

/**
 * Moves the iterator forward.
 *
 * @return  copy of this iterator before it was moved
 */
hat_trie::iterator
hat_trie::iterator::operator++(int) {
    iterator result = *this;
    operator++();
    return result;
}

/**
 * Moves the iterator backward.
 *
 * @return  self-reference
 */
hat_trie::iterator&
hat_trie::iterator::operator--() {
    return *this;
}

/**
 * Moves the iterator backward.
 *
 * @return  copy of this iterator before it was moved
 */
hat_trie::iterator
hat_trie::iterator::operator--(int) {
    iterator result = *this;
    operator--();
    return result;
}

/**
 * Iterator dereference operator.
 *
 * @return  string this iterator points to
 */
hat_trie::key_type
hat_trie::iterator::operator*() const {
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
bool
hat_trie::iterator::operator==(const iterator &rhs) {
    // TODO does iterator comparison need to be on more than just pointer?
    return _position == rhs._position;
}

/**
 * Overloaded not-equivalence operator.
 *
 * @param rhs  iterator to compare against
 * @return  true iff this iterator is not equal to @a rhs
 */
bool
hat_trie::iterator::operator!=(const iterator &rhs) {
    return !operator==(rhs);
}

/**
 * Special-purpose conversion constructor.
 *
 * If an iterator is constructed from a container pointer,
 * this function ensures that the iterator's internal iterator
 * across the elements in the container is properly initialized.
 */
hat_trie::
iterator::iterator(_node_pointer n) {
    operator=(n);
}

/**
 * Special-purpose assignment operator.
 *
 * If an iterator is assigned to a container pointer, this
 * function ensures that the iterator's internal iterator across
 * the elements in the container is properly initialized.
 */
hat_trie::iterator &
hat_trie::iterator::operator=(_node_pointer n) {
    this->_position = n;
    if (_position.type == CONTAINER_POINTER) {
        _container_iterator = ((_container *) _position.p)->_store.begin();
        _word = _position.p->word();
    }
    return *this;
}

/**
 * Namespace-scope swap function for hat tries.
 */
void swap(hat_trie &lhs, hat_trie &rhs) {
    lhs.swap(rhs);
}

// --------------------
// COMPARISON OPERATORS
// --------------------

bool
operator<(const stx::hat_trie &lhs, const stx::hat_trie &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
}
bool operator==(const stx::hat_trie &lhs, const stx::hat_trie &rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}
bool
operator>(const stx::hat_trie &lhs, const stx::hat_trie &rhs) {
    return rhs < lhs;
}
bool
operator<=(const stx::hat_trie &lhs, const stx::hat_trie &rhs) {
    return !(rhs < lhs);
}
bool
operator>=(const stx::hat_trie &lhs, const stx::hat_trie &rhs) {
    return !(lhs < rhs);
}
bool operator!=(const stx::hat_trie &lhs, const stx::hat_trie &rhs) {
    return !(lhs == rhs);
}

}  // namespace stx

namespace std {

/**
 * Template specialization of std::swap for hat_tries.
 */
template <>
void
swap(stx::hat_trie &lhs, stx::hat_trie &rhs) {
    lhs.swap(rhs);
}

}  // namespace std

