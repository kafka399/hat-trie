#include <iostream>

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

hat_trie::
~hat_trie() {
    delete _root;
    _root = NULL;
}

/**
 * Searches for a word in the trie.
 *
 * @param s  word to search for
 *
 * @return  true iff @a s is in the trie
 * @throws unindexed_character
 *      if a character in @a s is not indexed by @a indexof()
 */
bool
hat_trie::contains(const key_type &s) const {
    const char *ps = s.c_str();
    node_pointer n;
    return _search(ps, n);
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
 * @throws unindexed_character
 *      if a character in @a word is not indexed by @a indexof()
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
 * @throws unindexed_character
 *      if a character in @a word is not indexed by @a indexof()
 */
bool
hat_trie::insert(const char *word) {
    // Search for s in the trie.
    const char *pos = word;
    node_pointer n;
    bool found = _search(pos, n);
    if (!found) {
        // Was s found in the structure of the trie?
        if (*pos == '\0') {
            // s was found in the trie's structure. Mark its location
            // as the end of a word.
            if (n.pointer->is_word() == false) {
                n.pointer->set_word(true);
                ++_size;
            }

        } else {
            // s was not found in the trie's structure. Either make a
            // new container for it or insert it into an already
            // existing container.
            container *c = NULL;
            if (n.type == NODE_POINTER) {
                // Make a new container for s.
                node *p = (node *)n.pointer;
                int index = *pos;
                c = new container(index);

                // Insert the new container into the trie structure.
                c->parent = p;
                p->children[index] = c;
                p->types[index] = CONTAINER_POINTER;
                ++pos;
            } else if (n.type == CONTAINER_POINTER) {
                // The container for s already exists.
                c = (container *) n.pointer;
            }

            // Insert s into the container we found.
            return _insert(c, pos);
        }
    }

    return !found;
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
    result = _least(_root, result.cached_word);
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
    node_pointer n;

    // Search for the word in the trie.
    iterator result;
    if (_search(ps, n)) {
        // Initialize result to node n in the trie.
        result = n;
        result.cached_word = key_type(s.c_str(), ps);
        if (*ps != '\0') {
            // TODO initialize result.contanier_iterator too
            result.word = false;
            result.container_iterator = ((container *) n.pointer)->store.find(ps);
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
hat_trie::swap(self &rhs) {
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
    _root = new node();
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
bool
hat_trie::_search(const char *&s, node_pointer &n) const {
    // Search for a s in the trie.
    // Traverse the trie until either a s is used up, a is is found
    // in the trie, or s cannot be in the trie.
    node *p = _root;
    node_base *v = NULL;
    while (*s) {
        int index = *s;
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
bool
hat_trie::_insert(container *htc, const char *s) {
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
hat_trie::_burst(container *htc) {
    // Construct a new node.
    node *result = new node(htc->ch());
    result->set_word(htc->is_word());

    // Make a set of containers for the data in the old container and
    // add them to the new node.
    ht_array_hash::iterator it;
    for (it = htc->store.begin(); it != htc->store.end(); ++it) {
        int index = (*it)[0];

        // Do we need to make a new container?
        if (result->children[index] == NULL) {
            // Make a new container and position it under the new node.
            container *insertion = new container((*it)[0]);
            insertion->parent = result;
            result->children[index] = insertion;
            result->types[index] = CONTAINER_POINTER;

            // Set the new container's word field.
            insertion->set_word(((*it)[1] == '\0'));
        }

        // Insert the rest of the word into a container.
        if ((*it)[1] != '\0') {
            // Insert the rest of the word into the right container.
            ((container *) result->children[index])->insert((*it) + 1);
        } else {
            // Mark this container as a word.
            ((container *) result->children[index])->set_word(true);
        }
    }

    // Position the new node in the trie.
    node *parent = htc->parent;
    result->parent = parent;
    int index = htc->ch();
    parent->children[index] = result;
    parent->types[index] = NODE_POINTER;
    delete htc;
}

/**
 * Recursively prints the contents of the trie.
 */
void
hat_trie::_print(std::ostream &out,
                 const node_pointer &n,
                 const key_type &space) const {
    if (n.type == CONTAINER_POINTER) {
        container *c = (container *) n.pointer;
        if (c->ch() != '\0') {
            out << space << c->ch();
            if (c->is_word()) {
                out << " ~";
            }
            out << std::endl;
        }

        ht_array_hash::iterator it;
        it = c->store.begin();
        for (it = c->store.begin(); it != c->store.end(); ++it) {
            out << space + "  " << *it << " ~" << std::endl;
        }

    } else if (n.type == NODE_POINTER) {
        node *p = (node *)n.pointer;
        if (p->ch() != '\0') {
            out << space << p->ch();
            if (p->types[HT_ALPHABET_SIZE]) {
                out << " ~";
            }
            out << std::endl;
        }
        for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
            if (p->children[i]) {
                _print(out, node_pointer(p->types[i], p->children[i]), space + "  ");
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
hat_trie::node_pointer
hat_trie::_next_child(node *p, size_t pos, key_type &word) {
    node_pointer result;

    // Search for the next child under this node starting at pos.
    for (int i = pos; i < HT_ALPHABET_SIZE && result.pointer == NULL; ++i) {
        if (p->children[i]) {
            // Found a child.
            result.pointer = p->children[i];
            result.type = p->types[i];

            // Add this motion to the word.
            word += result.pointer->ch();
        }
    }
    return result;
}

/**
 * Finds the lexicographically least child under a node.
 *
 * @param p  parent node to search under
 * @param pos  starting position in the children array
 * @param word  cached word in the trie traversal
 * @return  a pointer to the next child under this node starting from
 *          @a pos, or NULL if this node has no children
 */
hat_trie::node_pointer
hat_trie::_least_child(node *p, key_type &word) {
    return _next_child(p, 0, word);
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
hat_trie::node_pointer
hat_trie::_next_word(node_pointer n, key_type &word) {
    // Stop early if we get a NULL pointer.
    if (n.pointer == NULL) { return node_pointer(); }

    node_pointer result;
    if (n.type == NODE_POINTER) {
        // Move to the leftmost child under this node.
        result = _least_child((node *) n.pointer, word);
    }

    if (result.pointer == NULL) {
        // This node has no children. Move up in the trie until
        // we can move right.
        node_pointer next;
        int pos;
        while (n.pointer->parent && next.pointer == NULL) {
            // Looks like we can't move to the right. Move up a level
            // in the trie and try again.
            pos = _pop_back(word) + 1;
            next = _next_child(n.pointer->parent, pos, word);
            n = n.pointer->parent;
        }
        result = next;
    }

    // Return the lexicographically least node underneath this one.
    return _least(result, word);
}

/**
 * Finds the lexicographically least node starting from @a n.
 *
 * @param n  current position in the trie
 * @param word  cached word in the trie traversal
 * @return  lexicographically least node from @a n. This function
 *          may return @a n itself
 */
hat_trie::node_pointer
hat_trie::_least(node_pointer n, key_type &word) {
    while (n.pointer && n.pointer->is_word() == false && n.type == NODE_POINTER) {
        // Find the leftmost child of this node and move in that direction.
        n = _least_child((node *) n.pointer, word);
    }
    return n;
}

/**
 * Removes a record from the back of a word.
 *
 * This function assumes word is populated. The string class
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
    if (n.type == CONTAINER_POINTER) {
        // If word is set, then this container represents a word as
        // well as storing words. The first iteration into the
        // container is over the word represented by the container.
        if (word) {
            // Move into the actual container by setting word to false.
            word = false;
        } else {
            // Move the iterator over the container's elements forward.
            ++container_iterator;
        }

        // If we aren't at the end of the container, stop here.
        if (container_iterator != ((container *) n.pointer)->store.end()) {
            return *this;
        }
    }

    // Move to the next node in the trie.
    return (*this = hat_trie::_next_word(n, cached_word));
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
    if (word || n.type == NODE_POINTER) {
        // Print the word that has been cached over the trie traversal.
        return cached_word;

    } else if (n.type == CONTAINER_POINTER) {
        // Pull a word from the container.
        return cached_word + *container_iterator;
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
    return n == rhs.n;
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
hat_trie::iterator::iterator(node_pointer n) {
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
hat_trie::iterator::operator=(node_pointer n) {
    this->n = n;
    if (n.type == CONTAINER_POINTER) {
        container_iterator = ((container *) n.pointer)->store.begin();
        word = n.pointer->is_word();
    }
    return *this;
}

/**
 * Namespace-scope swap function for hat tries.
 */
void swap(hat_trie &lhs, hat_trie &rhs) {
    lhs.swap(rhs);
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

