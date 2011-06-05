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
 * along with this program.  If not, see .
 */

#ifndef HAT_TRIE_NODE_H
#define HAT_TRIE_NODE_H

#include <bitset>

#include "array_hash.h"

namespace stx {

// number of distinct characters a hat trie can store
const int HT_ALPHABET_SIZE = 128;

// forward declarations
class hat_trie_traits;
template <class T> class hat_trie;
class hat_trie_container;
class hat_trie_node;

/**
 * Base class for hat trie node types.
 */
class hat_trie_node_base {
    friend class hat_trie<std::string>;

  private:
    typedef hat_trie_node _node;
    typedef hat_trie_container _container;

  public:
    hat_trie_node_base(char ch = '\0') : _ch(ch), _parent(NULL) { }
    virtual ~hat_trie_node_base() { }

    // accessors
    char ch() const { return _ch; }
    virtual bool word() const = 0;

    // modifiers
    virtual void set_word(bool b) = 0;

  protected:
    char _ch;
    _node *_parent;
};

/**
 * HAT-trie container type.
 *
 * Most of the words in a HAT-trie are stored in array hash tables. This
 * class is a wrapper around an array hash table that makes it look like
 * a node in a HAT-trie. Technically, the only functionality added to
 * the hash tables is a "word" element -- a boolean value that indicates
 * whether the container itself represents a word.
 */
class hat_trie_container : public hat_trie_node_base {
    friend class hat_trie<std::string>;

  public:
    /// Default constructor.
    hat_trie_container(char ch = '\0',
            const array_hash_traits &ah_traits = array_hash_traits()) :
            hat_trie_node_base(ch), _store(ah_traits) {
        set_word(false);
    }

    /// Destructor.
    ~hat_trie_container() { }

    /**
     * Determines whether the given string exists inside this container.
     *
     * @a p can either appear in the array hash table inside this container
     * or it can be represented by this container itself.
     *
     * @param p  string to check
     */
    bool exists(const char *p) const {
        if (*p == '\0') {
            // p is represented by this container
            return word();
        }
        // p is inside the internal array hash table
        return _store.exists(p);
    }

    /// Accessor for the size field inside the internal array hash table
    size_t size() const { return _store.size(); }

    /// Getter for the word field inside this container
    bool word() const { return _word; }

    /// Setter  for the word field inside this container
    void set_word(bool b) { _word = b; }

    /**
     * Inserts a word into this container.
     *
     * If @a p is an empty string, this container should represent it.
     * Otherwise, @a p should be passed down to the internal array hash
     * table.
     *
     * @param p  word to insert
     * @return  true if the word was successfully inserted, false if the
     *          word was already in this container
     */
    bool insert(const char *p) {
        if (*p == '\0') {
            bool b = word();
            set_word(true);
            return !b;
        }
        return _store.insert(p);
    }

    /**
     * Erases a word from this container.
     *
     * If @a p is an empty string, this container represents it. Otherwise,
     * it could be inside the internal array hash table.
     *
     * @param p  word to erase
     * @return  number of words that were erased. In distinct containers,
     *          this is either 0 or 1.
     */
    size_t erase(const char *p) {
        if (*p == '\0') {
            bool b = word();
            set_word(false);
            return b ? 1 : 0;
        }
        return _store.erase(p);
    }

    /**
     * Erases a word from this container.
     *
     * @param pos  iterator to a word inside the internal array hash table
     */
    void erase(const array_hash<std::string>::iterator &pos) {
        _store.erase(pos);
    }

  private:
    bool _word;
    array_hash<std::string> _store;
};

/**
 * HAT-trie node type.
 *
 * Represents a node in the HAT-trie hierarchy of nodes. Nodes can
 * represent a word and have children.
 */
class hat_trie_node : public hat_trie_node_base {
    friend class hat_trie<std::string>;

  private:
    typedef hat_trie_node_base _node_base;

  public:
    /// Default constructor.
    hat_trie_node(char ch = '\0') : _node_base(ch) {
        for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
            _children[i] = NULL;
        }
        set_word(false);
    }

    /// Destructor. Recursively deletes all the active nodes underneath
    /// this node.
    ~hat_trie_node() {
        for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
            delete _children[i];
        }
    }

    /// Getter for the word field inside this node
    bool word() const { return _types[HT_ALPHABET_SIZE]; }

    /// Setter for the word field inside this node
    void set_word(bool b) { _types[HT_ALPHABET_SIZE] = b; }

  private:
    std::bitset<HT_ALPHABET_SIZE + 1> _types;  // +1 is an end of word flag
    _node_base *_children[HT_ALPHABET_SIZE];  // untyped pointers to children
};

}  // namespace stx

#endif // HAT_TRIE_NODE_H

