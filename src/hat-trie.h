#ifndef HAT_TRIE_H
#define HAT_TRIE_H

#include <bitset>
#include <set>
#include <string>
#include <utility>

using namespace std;

namespace vaszauskas {

template <int AlphabetSize, int (*indexof)(char)>
class hat_trie;

}

namespace {

template <int AlphabetSize, int (*indexof)(char)>
class hat_trie_node;

template <int AlphabetSize, int (*indexof)(char)>
class hat_trie_container;

template <int AlphabetSize, int (*indexof)(char)>
class hat_trie_container {
    friend class vaszauskas::hat_trie<AlphabetSize, indexof>;

  private:
    typedef hat_trie_node<AlphabetSize, indexof> node;

  public:
    hat_trie_container(char ch = '\0');
    virtual ~hat_trie_container();

    bool contains(const string& s);
    void insert(const string& s);
    size_t size() const;

  private:
    char ch;
    bool word;
    node *parent;
    set<string> container;
};

template <int AlphabetSize, int (*indexof)(char)>
class hat_trie_node {
    friend class vaszauskas::hat_trie<AlphabetSize, indexof>;

  private:
    typedef hat_trie_container<AlphabetSize, indexof> container;
    typedef hat_trie_node<AlphabetSize, indexof> node;

  public:
    hat_trie_node(char ch = '\0');
    ~hat_trie_node();

  private:
    char ch;
    void *children[AlphabetSize];     // untyped pointers to children
    // To keep track of pointer types. The extra bit is an end-of-string flag.
    std::bitset<AlphabetSize + 1> types;
    node *parent;
};

}  // unnamed namespace

namespace vaszauskas {

template <int AlphabetSize, int (*indexof)(char)>
class hat_trie {

  private:
    typedef hat_trie_node<AlphabetSize, indexof> node;
    typedef hat_trie_container<AlphabetSize, indexof> container;

  public:
    hat_trie();
    virtual ~hat_trie();

    void insert(const string& s);

  public:
    size_t _size;  // number of distinct elements in the trie
    void *root;    // root of the trie
    char type;     // pointer type of root

    // constant values for the hat trie
    enum { CONTAINER_POINTER, NODE_POINTER };
    enum { BUCKET_SIZE_THRESHOLD = 4 };

    void init();
    bool search(const string& s);
    bool search(const string& s, pair<void *, int> &p);
    void burst(container *htc);
};

}  // namespace vaszauskas

namespace {

template <int AlphabetSize, int (*indexof)(char)>
hat_trie_container<AlphabetSize, indexof>::
hat_trie_container(char ch) : ch(ch), word(false), parent(NULL) {

}

template <int AlphabetSize, int (*indexof)(char)>
hat_trie_container<AlphabetSize, indexof>::
~hat_trie_container() {

}

template <int AlphabetSize, int (*indexof)(char)>
bool hat_trie_container<AlphabetSize, indexof>::
contains(const string& s) {
    return container.find(s) != container.end();
}

template <int AlphabetSize, int (*indexof)(char)>
void hat_trie_container<AlphabetSize, indexof>::
insert(const string& s) {
    container.insert(s);
}

template <int AlphabetSize, int (*indexof)(char)>
size_t hat_trie_container<AlphabetSize, indexof>::
size() const {
    return container.size();
}

template <int AlphabetSize, int (*indexof)(char)>
hat_trie_node<AlphabetSize, indexof>::
hat_trie_node(char ch) : ch(ch), parent(NULL) {
    for (int i = 0; i < AlphabetSize + 1; ++i) {
        children[i] = NULL;
    }
}

template <int AlphabetSize, int (*indexof)(char)>
hat_trie_node<AlphabetSize, indexof>::
~hat_trie_node() {

}

}  // anonymous namespace

namespace vaszauskas {

template <int AlphabetSize, int (*indexof)(char)>
hat_trie<AlphabetSize, indexof>::hat_trie() {
    init();
}

template <int AlphabetSize, int (*indexof)(char)>
hat_trie<AlphabetSize, indexof>::~hat_trie() {
    if (type == CONTAINER_POINTER) { delete (container *)root; }
    else if (type == NODE_POINTER) { delete (node *)root; }
    root = NULL;
}

template <int AlphabetSize, int (*indexof)(char)>
void hat_trie<AlphabetSize, indexof>::init() {
    _size = 0;
    root = new container();
    type = CONTAINER_POINTER;
}

template <int AlphabetSize, int (*indexof)(char)>
bool hat_trie<AlphabetSize, indexof>::
search(const string& s) {
    pair<void *, int> p;
    return search(s, p);
}

template <int AlphabetSize, int (*indexof)(char)>
bool hat_trie<AlphabetSize, indexof>::
search(const string& s, pair<void *, int>& p) {
    // Search for @a s in the tree.
    if (type == CONTAINER_POINTER) {
        container *htc = (container *)root;
        p = pair<void *, int>(root, CONTAINER_POINTER);
        return htc->contains(s);

    } else {
        node *n = (node *)root;
        void *v = NULL;
        // Traverse the trie until either @a s is used up, @a s is found in
        // the trie, or @a s is not found in the trie.
        size_t pos = 0;
        while (pos != s.length()) {
            int index = indexof(s[pos]);
            v = n->children[index];
            if (v) {
                if (n->types[index] == NODE_POINTER) {
                    n = (node *)v;
                } else if (n->types[index] == CONTAINER_POINTER) {
                    // Found a container that should contain @a s.
                    p = pair<void *, int>(v, CONTAINER_POINTER);
                    return ((container *)v)->word ||
                           ((container *)v)->contains(s.substr(pos + 1));
                }
            }
            ++pos;
        }
        // If we get here, no container was found that could have held @a s,
        // meaning @a s should appear at node @a n in the trie. Return true
        // iff the end of string flag in @a n is set.
        p = pair<void *, int>(n, NODE_POINTER);
        return n->types[AlphabetSize];
    }
}

template <int AlphabetSize, int (*indexof)(char)>
void hat_trie<AlphabetSize, indexof>::insert(const string& s) {
    if (type == CONTAINER_POINTER) {
        // Insert into the container root points to.
        container *htc = (container *)root;
        htc->insert(s);
        if (htc->size() > BUCKET_SIZE_THRESHOLD) {
            burst(htc);
        }

    } else if (type == NODE_POINTER) {
        // Find the location in the trie that the new word should be inserted
        // into.
        pair<void *, int> p;
        if (search(s, p)) {
            cout << "found " << s << " in the trie" << endl;
        } else {
            // @a s wasn't found in the trie.
        }
    }
}

template <int AlphabetSize, int (*indexof)(char)>
void hat_trie<AlphabetSize, indexof>::
burst(container *htc) {
    // Construct new node.
    int index = indexof(htc->ch);
    node *result = new node(htc->ch);
    node *parent = htc->parent;
    result->parent = parent;

    // Position new node in the tree.
    if (parent) {
        parent->children[index] = result;
        parent->types[index] = NODE_POINTER;
    } else {
        cout << "set root node to new node" << endl;
        root = result;
        type = NODE_POINTER;
    }

    // Make a set of containers for the data in the old container and add them
    // to the new node.
    set<string>::iterator it;
    for (it = htc->container.begin(); it != htc->container.end(); ++it) {
        int i = indexof((*it)[0]);
        if (result->children[i] == NULL) {
            container *insertion = new container((*it)[0]);
            insertion->word = it->length() == 1;
            result->children[i] = insertion;
            result->types[i] = CONTAINER_POINTER;
        }
        ((container *)result->children[i])->insert(it->substr(1));
    }

    cout << "Successful burst:" << endl;
    for (int i = 0; i < AlphabetSize; ++i) {
        if (result->children[i]) {
            cout << i << ": ";
            it = ((container *)result->children[i])->container.begin();
            while (it != ((container *)result->children[i])->container.end()) {
                cout << *it << " ";
                ++it;
            }
            cout << endl;
        }
    }
    cout << endl;
}

}  // namespace vaszauskas

#endif  // HAT_TRIE_H

