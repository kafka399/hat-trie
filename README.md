# Hat Trie
This project is intended to be a fully operational, standards compliant
HAT-trie. It will eventually mirror the STL set interface, but for now,
it is still _VERY_ much a work in progress.

## Working
Here is a list of the major operations that are working:

* ``insert(string)``
* ``contains(string)``
* ``find(string)``
* forward iteraton and iterator dereferencing

## Usage

### Installation
Copy all the headers into a directory in your PATH and include *hat-trie.h* in
your project. Note: some of the headers require ``inttypes.h``, which isn't
available by default on Windows platforms. You can find a compatible version
of the header on Google.

### Declaration
hat\_trie objects take two template parameters:

* size of the alphabet (defined as the set of possible characters)
* ``indexof(char)`` function that indexes characters in the alphabet

``indexof()`` should return an integer in [0, ``alphabet_size``) that is
unique for each character in the alphabet. 

Here is an example ``indexof()`` function that indexes alphanumeric
characters:

    int alphanumeric_index(char ch) {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        return ch - 'a' + 10;
    }

In this case, the first 10 index values are the characters 0-9, and the next
26 index values are the characters a-z. Note that any value outside the range
[0, ``alphabet_size``) indicates an invalid character. If a hat\_trie finds 
an invalid character, an ``unindexed_character`` exception is thrown. 

To declare a hat\_trie that supports this alphabet:

    hat_trie<36, alphanumeric_index> ht;
    ht.insert(...);


## Extensions
There are a few non-standard extensions to the hat\_trie interface:

* ``contains(string)`` -- returns true iff the string is in the trie

## Deviations
The hat\_trie interface differs from the standard in a few ways:

* ``insert(string)`` -- returns a ``bool`` rather than a 
  ``pair<iterator, bool>``
