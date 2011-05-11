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

## Extensions
There are a few non-standard extensions to the hat\_trie interface:

* ``contains(string)`` -- returns true iff the string is in the trie

## Deviations
The hat\_trie interface differs from the standard in a few ways:

* ``insert(string)`` -- returns a ``bool`` rather than a 
  ``pair<iterator, bool>``
