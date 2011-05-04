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

// TODO exclude these classes / this file from Doxygen
// TODO doc comments

#ifndef HAT_TRIE_NODE_H
#define HAT_TRIE_NODE_H

#include <bitset>

#include "array-hash.h"

namespace stx {

class hat_trie;

class hat_trie_container;

class hat_trie_node;

class hat_trie_node_base {
    friend class hat_trie;

  private:
    typedef hat_trie_node node;
    typedef hat_trie_container container;

  public:
    hat_trie_node_base(char ch = '\0') : _ch(ch), parent(NULL) { }
    virtual ~hat_trie_node_base() { }

    // accessors
    char ch() const { return _ch; }
    virtual bool is_word() const = 0;

    // modifiers
    virtual void set_word(bool b) = 0;

  protected:
    char _ch;
    node *parent;
};

// -----------------------
// hat trie helper classes
// -----------------------

class hat_trie_container : public hat_trie_node_base {
    friend class hat_trie;

  public:
    hat_trie_container(char ch = '\0');
    virtual ~hat_trie_container() { }

    // accessors
    bool contains(const char *p) const;
    size_t size() const { return store.size(); }
    bool is_word() const { return word; }

    // modifiers
    bool insert(const char *p);
    void set_word(bool b) { word = b; }

  private:
    bool word;
    ht_array_hash store;
};

class hat_trie_node : public hat_trie_node_base {
    friend class hat_trie;

  private:
    typedef hat_trie_node_base node_base;

  public:
    hat_trie_node(char ch = '\0');
    virtual ~hat_trie_node();

    // accessors
    bool is_word() const { return types[HT_ALPHABET_SIZE]; }

    // modifiers
    void set_word(bool b) { types[HT_ALPHABET_SIZE] = b; }

  private:
    std::bitset<HT_ALPHABET_SIZE + 1> types;  // extra bit is an end of word flag
    node_base *children[HT_ALPHABET_SIZE];  // untyped pointers to children
};

}  // namespace stx

#endif // HAT_TRIE_NODE_H

