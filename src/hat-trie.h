// TODO
// what if bursting gives you a container that still has more elements than
// BURST_THRESHOLD? I think bursting this one lazily (on the next time you
// insert into this container) is acceptable.

#ifndef HAT_TRIE_H
#define HAT_TRIE_H

#include <bitset>
#include <exception>
#include <set>
#include <string>
#include <utility>

using namespace std;

namespace vaszauskas {

template <int AlphabetSize, int (*indexof)(char)>
class hat_trie;

// Exception class for bad index values.
class bad_index : public exception {
    virtual const char *what() const throw() {
        return "Invalid index value from indexof() function.";
    }
};

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

  public:
    char ch;
    void *children[AlphabetSize];  // untyped pointers to children
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
    void print(void *p, int type, const string& space = "") {
        if (type == CONTAINER_POINTER) {
            container *c = (container *)p;
            set<string>::iterator sit;
            if (c->ch != '\0') {
                cout << space << c->ch;
                if (c->word) {
                    cout << " ~";
                }
                cout << endl;
            }
            for (sit = c->container.begin(); sit != c->container.end(); ++sit) {
                assert(*sit != "");
                cout << space + "  " << *sit << " ~" << endl;
            }
        } else if (type == NODE_POINTER) {
            node *n = (node *)p;
            if (n->ch != '\0') {
                cout << space << n->ch;
                if (n->types[AlphabetSize]) {
                    cout << " ~";
                }
                cout << endl;
            }
            for (int i = 0; i < AlphabetSize; ++i) {
                if (n->children[i]) {
                    print(n->children[i], n->types[i], space + "  ");
                }
            }
        }
    }

  public:
    size_t _size;  // number of distinct elements in the trie
    void *root;    // root of the trie
    char type;     // pointer type of root

    // constant values for the hat trie
    enum { CONTAINER_POINTER, NODE_POINTER };
    enum { BUCKET_SIZE_THRESHOLD = 512 };

    void init();
    int getindex(char ch) throw(bad_index);
    bool search(const string& s);
    bool search(const string& s, pair<void *, int> &p, size_t& pos);
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
    for (int i = 0; i < AlphabetSize; ++i) {
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
int hat_trie<AlphabetSize, indexof>::getindex(char ch) throw(bad_index) {
    int result = indexof(ch);
    if (result < 0 || result >= AlphabetSize) {
        throw bad_index();
    }
    return result;
}

template <int AlphabetSize, int (*indexof)(char)>
bool hat_trie<AlphabetSize, indexof>::
search(const string& s) {
    pair<void *, int> p;
    size_t pos;
    return search(s, p, pos);
}

template <int AlphabetSize, int (*indexof)(char)>
bool hat_trie<AlphabetSize, indexof>::
search(const string& s, pair<void *, int>& p, size_t& pos) {
    // Search for @a s in the tree.
    if (type == CONTAINER_POINTER) {
        container *htc = (container *)root;
        p = pair<void *, int>(root, CONTAINER_POINTER);
        pos = s.length();
        return htc->contains(s);

    } else if (type == NODE_POINTER) {
        node *n = (node *)root;
        void *v = NULL;
        pos = 0;
        // Traverse the trie until either @a s is used up, @a s is found in
        // the trie, or @a s is not found in the trie.
        while (pos < s.length()) {
            int index = getindex(s[pos]);
            v = n->children[index];
            if (v) {
                if (n->types[index] == NODE_POINTER) {
                    n = (node *)v;
                    //cout << "followed node path by " << s[pos] << endl;
                } else if (n->types[index] == CONTAINER_POINTER) {
                    //cout << "found container pointer at " << s[pos] << endl;
                    if (pos == s.length() - 1) {
                        // This node represents @a s.
                        p = pair<void *, int>(v, CONTAINER_POINTER);
                        ++pos;
                        //cout << "return here" << endl;
                        return ((container *)v)->word;
                    } else {
                        //cout << "found existing container" << endl;
                        // @a s should appear in the container represented by
                        // @a v.
                        p = pair<void *, int>(v, CONTAINER_POINTER);
                        if (pos == s.length()) {
                            return ((container *)v)->word;
                        } else {
                            return ((container *)v)->contains(s.substr(pos + 1));
                        }
                    }
                }
            } else {
                p = pair<void *, int>(n, NODE_POINTER);
                return false;
            }
            ++pos;
        }
        // If we get here, no container was found that could have held @a s,
        // meaning @a s should appear at node @a n in the trie. Return true
        // iff the end of string flag in @a n is set.
        p = pair<void *, int>(n, NODE_POINTER);
        return n->types[AlphabetSize];
    }
    return false;
}

template <int AlphabetSize, int (*indexof)(char)>
void hat_trie<AlphabetSize, indexof>::insert(const string& s) {
    //cout << "INSERTING " << s << endl;
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
        size_t pos;
        if (search(s, p, pos)) {
            //cout << "found " << s << " in the trie, so didn't insert" << endl;
        } else {
            // @a s wasn't found in the trie. Insert it.
            if (p.second == NODE_POINTER) {
                //cout << "p.second is node pointer" << endl;
                node *n = (node *)p.first;
                if (pos == s.length()) {
                    //cout << "pos == s.length()" << endl;
                    // Just set the node's end of word flag to true.
                    n->types.set(AlphabetSize);
                } else {
                    //cout << "pos != s.length(): " << pos << endl;
                    // A new container needs to be made to accomodate @a s.
                    container *c = new container(s[pos]);
                    c->parent = n;
                    if (pos + 1 == s.length()) {
                        // The new container itself represents @a s.
                        c->word = true;
                    } else {
                        // Add the remainder of @a s to the new container.
                        c->insert(s.substr(pos + 1));
                    }
                    int index = getindex(s[pos]);
                    n->children[index] = c;
                    n->types[index] = CONTAINER_POINTER;
                }
            } else if (p.second == CONTAINER_POINTER) {
                //cout << "p.second is container pointer" << endl;
                // The word needs to be added to the trie.
                container *c = (container *)p.first;
                if (pos == s.length()) {
                    //cout << "pos == s.length()" << endl;
                    // Set the container's word flag to true.
                    c->word = true;
                } else {
                    //cout << "pos != s.length()" << endl;
                    // Insert the leftover part of @a s into the container.
                    c->insert(s.substr(pos + 1));
                    if (c->size() > BUCKET_SIZE_THRESHOLD) {
                        burst(c);
                    }
                }
            }
        }
    }
}

template <int AlphabetSize, int (*indexof)(char)>
void
hat_trie<AlphabetSize, indexof>::burst(container *htc) {
    //cout << "BURSTING " << endl;
    // Construct new node.
    node *result = new node(htc->ch);
    result->types[AlphabetSize] = htc->word;

    // Make a set of containers for the data in the old container and add them
    // to the new node.
    set<string>::iterator it;
    for (it = htc->container.begin(); it != htc->container.end(); ++it) {
        int index = getindex((*it)[0]);
        if (result->children[index] == NULL) {
            container *insertion = new container((*it)[0]);
            insertion->word = it->length() == 1;
            insertion->parent = result;
            result->children[index] = insertion;
            result->types[index] = CONTAINER_POINTER;
        }
        if (it->length() > 1) {
            ((container *)result->children[index])->insert(it->substr(1));
        }
    }

    // Position the new node in the tree.
    node *parent = htc->parent;
    result->parent = parent;
    if (parent) {
        int index = getindex(htc->ch);
        parent->children[index] = result;
        parent->types[index] = NODE_POINTER;
    } else {
        root = result;
        type = NODE_POINTER;
    }
    delete htc;
}

}  // namespace vaszauskas

#endif  // HAT_TRIE_H

