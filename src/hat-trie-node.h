#ifndef HAT_TRIE_NODE_H
#define HAT_TRIE_NODE_H

#include <cassert>

namespace stx {

template <int alphabet_size, int (*indexof)(char)>
class hat_trie;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_container;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node;

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node_base {
    friend class stx::hat_trie<alphabet_size, indexof>;

  private:
    typedef hat_trie_node<alphabet_size, indexof> node;
    typedef hat_trie_container<alphabet_size, indexof> container;

  public:
    hat_trie_node_base(char ch = '\0');
    virtual ~hat_trie_node_base() { };

    // accessors
    virtual bool word() const = 0;
    char ch() const { return _ch; }

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
    friend class stx::hat_trie<alphabet_size, indexof>;

  public:
    typedef stx::array_hash store_type;

    hat_trie_container(char ch = '\0');
    virtual ~hat_trie_container();

    // accessors
    bool contains(const char *p) const;
    size_t size() const;
    bool word() const;

    // modifiers
    bool insert(const char *p);
    void set_word(bool b);

  private:
    bool _word;
    stx::array_hash store;
};

template <int alphabet_size, int (*indexof)(char)>
class hat_trie_node {
    friend class stx::hat_trie<alphabet_size, indexof>;

  private:
    typedef hat_trie_container<alphabet_size, indexof> container;
    typedef hat_trie_node<alphabet_size, indexof> node;

  public:
    hat_trie_node(char ch = '\0');
    ~hat_trie_node();

    // accessors
    bool is_word() const;

    // modifiers
    void set_word(bool b);

  public:
    char ch;
    void *children[alphabet_size];  // untyped pointers to children
    // To keep track of pointer types. The extra bit is an end-of-string flag.
    std::bitset<alphabet_size + 1> types;
    node *parent;
};

// ---------------------------------
// hat_trie_node_base implementation
// ---------------------------------
template <int alphabet_size, int (*indexof)(char)>
hat_trie_node_base<alphabet_size, indexof>::
hat_trie_node_base(char ch) : _ch(ch), parent(NULL) {

}

// ---------------------------------
// hat_trie_container implementation
// ---------------------------------

template <int alphabet_size, int (*indexof)(char)>
hat_trie_container<alphabet_size, indexof>::
hat_trie_container(char ch) : _word(false) {
    assert(_ch == ch);
}

template <int alphabet_size, int (*indexof)(char)>
hat_trie_container<alphabet_size, indexof>::
~hat_trie_container() {

}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
contains(const char *p) const {
    if (*p == '\0') {
        return word();
    }
    return store.find(p);
}

template <int alphabet_size, int (*indexof)(char)>
size_t hat_trie_container<alphabet_size, indexof>::
size() const {
    return store.size();
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
word() const {
    return _word;
}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_container<alphabet_size, indexof>::
insert(const char *p) {
    if (*p == '\0') {
        bool b = word();
        set_word(true);
        return !b;
    }
    return store.insert(p);
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie_container<alphabet_size, indexof>::
set_word(bool b) {
    _word = b;
}

// ----------------------------
// hat_trie_node implementation
// ----------------------------

template <int alphabet_size, int (*indexof)(char)>
hat_trie_node<alphabet_size, indexof>::
hat_trie_node(char ch) : ch(ch), parent(NULL) {
    for (int i = 0; i < alphabet_size; ++i) {
        children[i] = NULL;
    }
}

template <int alphabet_size, int (*indexof)(char)>
hat_trie_node<alphabet_size, indexof>::
~hat_trie_node() {

}

template <int alphabet_size, int (*indexof)(char)>
bool hat_trie_node<alphabet_size, indexof>::
is_word() const {
    return types[alphabet_size];
}

template <int alphabet_size, int (*indexof)(char)>
void hat_trie_node<alphabet_size, indexof>::
set_word(bool val) {
    types[alphabet_size] = val;
}

}  // namespace stx

#endif // HAT_TRIE_NODE_H

