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

#include "array-hash.h"

namespace stx {

// number of distinct characters a hat trie can store
const int HT_ALPHABET_SIZE = 128;

// forward declarations
class hat_trie;
class hat_trie_container;
class hat_trie_node;

/**
 * Base class for hat trie node types.
 */
class hat_trie_node_base {
    friend class hat_trie;

  private:
    typedef hat_trie_node _node;
    typedef hat_trie_container _container;

  public:
    hat_trie_node_base(char ch = '\0') : _ch(ch), _parent(NULL) {
        set_word(false);
    }
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
 */
class hat_trie_container : public hat_trie_node_base {
    friend class hat_trie;

  public:
    hat_trie_container(char ch = '\0') : hat_trie_node_base(ch) { }
    ~hat_trie_container() { }

    // accessors
    bool contains(const char *p) const {
        if (*p == '\0') {
            return word();
        }
        return _store.contains(p);
    }
    size_t size() const { return _store.size(); }
    bool word() const { return _word; }

    // modifiers
    bool insert(const char *p) {
        if (*p == '\0') {
            bool b = word();
            set_word(true);
            return !b;
        }
        return _store.insert(p);
    }
    void set_word(bool b) { _word = b; }

  private:
    bool _word;
    array_hash _store;
};

/**
 * HAT-trie node type.
 */
class hat_trie_node : public hat_trie_node_base {
    friend class hat_trie;

  private:
    typedef hat_trie_node_base _node_base;

  public:
    hat_trie_node(char ch = '\0') : _node_base(ch) {
        for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
            _children[i] = NULL;
        }
    }
    ~hat_trie_node() {
        for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
            delete _children[i];
        }
    }

    // accessors
    bool word() const { return _types[HT_ALPHABET_SIZE]; }

    // modifiers
    void set_word(bool b) { _types[HT_ALPHABET_SIZE] = b; }

  private:
    std::bitset<HT_ALPHABET_SIZE + 1> _types;  // extra bit is an end of word flag
    _node_base *_children[HT_ALPHABET_SIZE];  // untyped pointers to children
};

}  // namespace stx

#endif // HAT_TRIE_NODE_H

