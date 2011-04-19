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
#include <stack>
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
    typedef hat_trie<alphabet_size, indexof> self;
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

        // Conversion constructor from node * to node_pointer type
        node_pointer(node *n) : type(NODE_POINTER), pointer(n) { }

        // comparison operators
        bool operator==(const node_pointer &rhs)
        { return pointer == rhs.pointer; }
        bool operator!=(const node_pointer &rhs)
        { return !operator==(rhs); }
    };

  public:
	class iterator;
    typedef iterator const_iterator;

    // constructors and destructors
    hat_trie();
    virtual ~hat_trie();

    // accessors
    bool contains(const std::string &s) const;
    bool empty() const;
    size_t size() const;
    void print() const { print(root); }

    // modifiers
    bool insert(const std::string &s);

    // iterators
    iterator begin() const;
    iterator end() const;
    iterator find(const std::string &s) const;

    // utilities
    void swap(self &rhs);

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
    class iterator : std::iterator<std::bidirectional_iterator_tag, node_base> {
        friend class hat_trie;

      public:
        iterator() { }

        iterator operator++(int);
        iterator &operator++();
        iterator operator--(int);
        iterator &operator--();

        std::string operator*() const;
        bool operator==(const iterator &rhs);
        bool operator!=(const iterator &rhs);

      private:
        // Current location in the trie
        node_pointer n;

        // Internal iterator across container types
        typename container::store_type::iterator container_iterator;
        bool word;

        // Caches the word as we move up and down the trie and
        // implicitly caches the path we followed as well
        std::string cached_word;

        // Special-purpose constructor and assignment operator. If
        // an iterator is assigned to a container, it automatically
        // initializes its internal iterator to the first element
        // in that container.
        iterator(node_pointer);
        iterator &operator=(node_pointer);

    };

  private:
    node *root;  // pointer to the root of the trie
    size_t _size;  // number of distinct elements in the trie

    // types node_base values could point to. This is stored in
    // one bit, so the only valid values are 0 and 1
    enum { NODE_POINTER = 0, CONTAINER_POINTER = 1 };

    // containers are burst after their size crosses this threshold
    // MUST be <= 32,768
    enum { BURST_THRESHOLD = 8192 };

    void init();

    // accessors
    bool search(const char * &s, node_pointer &n) const;
    void print(const node_pointer &n, const std::string &space = "") const;

    static node_pointer next_child(node *, size_t, std::string &);
    static node_pointer least_child(node *, std::string &);
    static node_pointer next_word(node_pointer, std::string &);
    static node_pointer least(node_pointer, std::string &);
    static int pop_back(std::string &);

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
 * @return  true iff @a s is in the trie
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
 * Determines whether this container is empty.
 *
 * @return  true iff this container has no data
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
empty() const {
    return size() == 0;
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
                int index = ht_get_index<alphabet_size, indexof>(*pos);
                c = new container(*pos);

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
    // Stop early if there are no elements in the trie.
    if (size() == 0) {
        return end();
    }

    // Incrementally construct the iterator to return. This code
    // is pretty ugly. See the doc comment for the iterator class
    // for a description of why.
    iterator result;
    result = least(root, result.cached_word);
    return result;
}

/**
 * Searches for @a s in the trie.
 *
 * @param s  word to search for
 * @return  iterator to @a s in the trie. If @a s is not in the trie,
 *          returns an iterator to one past the last element
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator
hat_trie<alphabet_size, indexof>::
find(const std::string &s) const {
    const char *ps = s.c_str();
    node_pointer n;

    // Search for the word in the trie.
    iterator result;
    if (search(ps, n)) {
        // Initialize result to node n in the trie.
        result = n;
        result.cached_word = std::string(s.c_str(), ps);
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
template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
swap(self &rhs) {
    using std::swap;
    swap(root, rhs.root);
    swap(_size, rhs._size);
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
 *       ust ~ (word in the container)
 *       ee ~ (word in the container)
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
    int index = ht_get_index<alphabet_size, indexof>(htc->ch());
    parent->children[index] = result;
    parent->types[index] = NODE_POINTER;
    delete htc;
}

/**
 * Recursively prints the contents of the trie.
 */
template <int alphabet_size, int (*indexof)(char)>
void hat_trie<alphabet_size, indexof>::
print(const node_pointer &n, const std::string &space) const {
    using namespace std;

    if (n.type == CONTAINER_POINTER) {
        container *c = (container *) n.pointer;
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
                print(node_pointer(p->types[i], p->children[i]), space + "  ");
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
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::node_pointer
hat_trie<alphabet_size, indexof>::
next_child(node *p, size_t pos, std::string &word) {
    node_pointer result;

    // Search for the next child under this node starting at pos.
    for (size_t i = pos; i < alphabet_size && result.pointer == NULL; ++i) {
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
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::node_pointer
hat_trie<alphabet_size, indexof>::
least_child(node *p, std::string &word) {
    return next_child(p, 0, word);
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
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::node_pointer
hat_trie<alphabet_size, indexof>::
next_word(node_pointer n, std::string &word) {
    // Stop early if we get a NULL pointer.
    if (n.pointer == NULL) { return node_pointer(); }

    node_pointer result;
    if (n.type == NODE_POINTER) {
        // Move to the leftmost child under this node.
        result = least_child((node *) n.pointer, word);
    }

    if (result.pointer == NULL) {
        // This node has no children. Move up in the trie until
        // we can move right.
        node_pointer next;
        int pos;
        while (n.pointer->parent && next.pointer == NULL) {
            // Looks like we can't move to the right. Move up a level
            // in the trie and try again.
            pos = pop_back(word) + 1;
            next = next_child(n.pointer->parent, pos, word);
            n = n.pointer->parent;
        }
        result = next;
    }

    // Return the lexicographically least node underneath this one.
    return least(result, word);
}

/**
 * Finds the lexicographically least node starting from @a n.
 *
 * @param n  current position in the trie
 * @param word  cached word in the trie traversal
 * @return  lexicographically least node from @a n. This function
 *          may return @a n itself
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::node_pointer
hat_trie<alphabet_size, indexof>::
least(node_pointer n, std::string &word) {
    while (n.pointer && n.pointer->is_word() == false && n.type == NODE_POINTER) {
        // Find the leftmost child of this node and move in that direction.
        n = least_child((node *) n.pointer, word);
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
template <int alphabet_size, int (*indexof)(char)>
int hat_trie<alphabet_size, indexof>::
pop_back(std::string &word) {
    int result = indexof(word[word.size() - 1]);
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
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator&
hat_trie<alphabet_size, indexof>::
iterator::operator++() {
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
    return (*this = hat_trie::next_word(n, cached_word));
}

/**
 * Moves the iterator forward.
 *
 * @return  copy of this iterator before it was moved
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator
hat_trie<alphabet_size, indexof>::
iterator::operator++(int) {
    iterator result = *this;
    operator++();
    return result;
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
    return *this;
}

/**
 * Moves the iterator backward.
 *
 * @return  copy of this iterator before it was moved
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator
hat_trie<alphabet_size, indexof>::
iterator::operator--(int) {
    iterator result = *this;
    operator--();
    return result;
}

/**
 * Iterator dereference operator.
 *
 * @return  string this iterator points to
 */
template <int alphabet_size, int (*indexof)(char)>
std::string hat_trie<alphabet_size, indexof>::
iterator::operator*() const {
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
 * @return  true iff this iterator points to the same thing as
 *          @a rhs
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
iterator::operator==(const iterator &rhs) {
    // TODO does iterator comparison need to be on more than just pointer?
    return n == rhs.n;
}

/**
 * Overloaded not-equivalence operator.
 *
 * @param rhs  iterator to compare against
 * @return  true iff this iterator is not equal to @a rhs
 */
template <int alphabet_size, int (*indexof)(char)>
bool hat_trie<alphabet_size, indexof>::
iterator::operator!=(const iterator &rhs) {
    return !operator==(rhs);
}

/**
 * Special-purpose conversion constructor.
 *
 * If an iterator is constructed from a container pointer,
 * this function ensures that the iterator's internal iterator
 * across the elements in the container is properly initialized.
 */
template <int alphabet_size, int (*indexof)(char)>
hat_trie<alphabet_size, indexof>::
iterator::iterator(node_pointer n) {
    operator=(n);
}

/**
 * Special-purpose assignment operator.
 *
 * If an iterator is assigned to a container pointer, this
 * function ensures that the iterator's internal iterator across
 * the elements in the container is properly initialized.
 */
template <int alphabet_size, int (*indexof)(char)>
typename hat_trie<alphabet_size, indexof>::iterator &
hat_trie<alphabet_size, indexof>::
iterator::operator=(node_pointer n) {
    this->n = n;
    if (n.type == CONTAINER_POINTER) {
        container_iterator = ((container *) n.pointer)->store.begin();
        word = n.pointer->is_word();
    }
    return *this;
}

}  // namespace stx

namespace std {

/**
 * Template specialization of std::swap for hat tries.
 */
template <int alphabet_size, int (*indexof)(char)>
void swap(stx::hat_trie<alphabet_size, indexof> &lhs,
          stx::hat_trie<alphabet_size, indexof> &rhs) {
    lhs.swap(rhs);
}

}  // namespace std

#endif  // HAT_TRIE_H

