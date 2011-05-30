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

#ifndef HAT_SET_H
#define HAT_SET_H

#include "hat_trie.h"

namespace stx {

/// Forward declaration of the hat_set class.
template <class T> class hat_set;

/**
 * HAT-trie set class that implements (most of) the STL set interface.
 *
 * Note: the only available template parameter is std::string. Using
 * any other template parameter will result in a compile-time error.
 */
template <>
class hat_set<std::string> {

  private:
    typedef hat_trie<std::string> hat_trie;
    typedef hat_set<std::string>  _self;

  public:
    // STL types
    typedef hat_trie::size_type     size_type;
    typedef hat_trie::key_type      key_type;
    typedef key_type                value_type;

    typedef hat_trie::iterator      iterator;
    typedef iterator                const_iterator;

    hat_set(const hat_trie_traits &traits = hat_trie_traits(),
            const array_hash_traits &ah_traits = array_hash_traits()) :
            trie(traits, ah_traits) { }

    /**
     * Searches for a word in the trie.
     *
     * @param word  word to search for
     * @return  true iff @a word is in the trie
     */
    bool exists(const key_type &word) const {
        return trie.exists(word);
    }

    /**
     * Determines whether this set is empty.
     *
     * @return  true iff the container has no data
     */
    bool empty() const {
        return trie.empty();
    }

    /**
     * Gets the number of elements in the trie.
     *
     * @return  number of elements in the trie
     */
    size_type size() const {
        return trie.size();
    }

    /**
     * Gets a const reference to the traits associated with this trie.
     *
     * @return  traits associated with this trie
     */
    const hat_trie_traits &traits() const {
        return trie.traits();
    }

    /**
     * Gets the array hash traits associated with the hash tables in
     * this trie.
     *
     * @return  array hash traits associated with this trie
     */
    const array_hash_traits &hash_traits() const {
        return trie.hash_traits();
    }

    /**
     * Removes all the elements in the trie.
     */
    void clear() {
        trie.clear();
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
        return trie.insert(word);
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
        return trie.insert(word);
    }

    /**
     * Inserts several words into the trie.
     *
     * @param first, last  iterators specifying a range of words to add
     *                     to the trie. All words in the range
     *                     [first, last) are added
     */
    template <class input_iterator>
    void insert(const input_iterator &first, const input_iterator &last) {
        trie.insert(first, last);
    }

    /**
     * Inserts several words into the trie.
     *
     * In standard STL sets, this function can dramatically increase
     * performance if @a position is set correctly. This performance
     * gain is unachievable in a HAT-trie because the time required to
     * verify that @a position points to the right place is just as
     * expensive as a regular insert operation.
     *
     * @param pos   unused
     * @param word  word to insert
     * @return iterator to @a word in the trie
     */
    iterator insert(const iterator &pos, const key_type &word) {
        return trie.insert(pos, word);
    }

    /**
     * Erases a word from the trie.
     *
     * @param pos  iterator to the word in the trie
     */
    void erase(const iterator &pos) {
        trie.erase(pos);
    }

    /**
     * Erases a word from the trie.
     *
     * @param word  word to erase
     * @return  number of words erased from the trie. In a set container,
     *          either 1 if the word was removed from the trie or 0 if the
     *          word doesn't appear in the trie
     */
    size_type erase(const key_type &word) {
        return trie.erase(word);
    }

    /**
     * Erases several words from the trie.
     *
     * @param first, last  iterators specifying a range of words to remove
     *                     from the trie. All words in the range [first,
     *                     last) are removed
     */
    void erase(const iterator &first, const iterator &last) {
        trie.erase(first, last);
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
        return trie.begin();
    }

    /**
     * Gets an iterator to one past the last element in the trie.
     *
     * @return iterator to one past the last element in the trie
     */
    iterator end() const {
        return trie.end();
    }

    /**
     * Searches for @a word in the trie.
     *
     * @param word  word to search for
     * @return  iterator to @a word in the trie, or @a end() if @a word
     *          is not found
     */
    iterator find(const key_type &word) const {
        return trie.find(word);
    }

    /**
     * Swaps the data in two hat_set objects.
     *
     * @param rhs  hat_set object to swap data with
     */
    void swap(hat_set &rhs) {
        trie.swap(rhs.trie);
    }

  private:
    hat_trie trie;

};

}  // namespace stx

#endif

