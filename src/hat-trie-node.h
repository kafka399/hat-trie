/*
 * Copyright 2010-2011 Chris Vaszauskas and Tyler Richard
 *
 * This file is part of a HAT-trie implementation.
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

// TODO exclude these classes / this file from Doxygen

#ifndef HAT_TRIE_NODE_H
#define HAT_TRIE_NODE_H

namespace stx {

template <int alphabet_size, int (*indexof)(char)>
class hat_trie;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_container;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node_base {
    friend class hat_trie<alphabet_size, indexof>;

  private:
    typedef hat_trie_node<alphabet_size, indexof> node;
    typedef hat_trie_container<alphabet_size, indexof> container;

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

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_container : public hat_trie_node_base<alphabet_size, indexof> {
    friend class hat_trie<alphabet_size, indexof>;

  public:
    typedef array_hash store_type;

    hat_trie_container(char ch = '\0');
    virtual ~hat_trie_container() { }

    // accessors
    bool contains(const char *p) const;
    size_t size() const { return store.size(); }
    bool is_word() const { return word; }
    store_type::iterator begin() const { return store.begin(); }

    // modifiers
    bool insert(const char *p);
    void set_word(bool b) { word = b; }

  private:
    bool word;
    array_hash store;
};

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node : public hat_trie_node_base<alphabet_size, indexof> {
    friend class hat_trie<alphabet_size, indexof>;

  private:
    typedef hat_trie_node_base<alphabet_size, indexof> node_base;

  public:
    hat_trie_node(char ch = '\0');
    ~hat_trie_node() { }

    // accessors
    bool is_word() const { return types[alphabet_size]; }

    // modifiers
    void set_word(bool b) { types[alphabet_size] = b; }

  private:
    std::bitset<alphabet_size + 1> types;  // extra bit is an end of word flag
    node_base *children[alphabet_size];  // untyped pointers to children
};

// ---------------------------------
// hat_trie_container implementation
// ---------------------------------

template <int alphabet_size, int (*indexof)(char)>
hat_trie_container<alphabet_size, indexof>::
hat_trie_container(char ch) :
        hat_trie_node_base<alphabet_size, indexof>(ch) {
    set_word(false);
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
contains(const char *p) const {
    if (*p == '\0') {
        return is_word();
    }
    return store.find(p);
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
insert(const char *p) {
    if (*p == '\0') {
        bool b = is_word();
        set_word(true);
        return !b;
    }
    return store.insert(p);
}

// ----------------------------
// hat_trie_node implementation
// ----------------------------

template <int alphabet_size, int (*indexof)(char)>
hat_trie_node<alphabet_size, indexof>::
hat_trie_node(char ch) :
        hat_trie_node_base<alphabet_size, indexof>(ch) {
    for (int i = 0; i < alphabet_size; ++i) {
        children[i] = NULL;
    }
}

}  // namespace stx

#endif // HAT_TRIE_NODE_H

