Evo C++ Library
---------------
Version: 0.5

Evo is a modern high performance C++ library designed to make writing efficient and portable code easy. It covers:

 - core types, strings, and containers
 - string tokenization and parsing
 - file/pipe/socket streams and formatting
 - threads and atomics
 - C++03 and C++11 (and newer)

For the latest version of Evo and online documentation see: https://github.com/jlctools/evo
Documentation is also included with this distribution.

Using Evo
---------

You may include all core Evo headers like the following or include individual headers as needed.
Note that Evo headers are best included in the form #include <evo/HEADER.h> (from the parent
directory). See Evo documentation for more detail.

A quick example.cpp to get you started:

```
#include <evo/core.h>
using namespace evo;

int main() {
    Console& c = con();
    c.out << "Hello Evo " << 123 << '!' << NL;

    String str;
    str << "Test String " << 123;
    c.out << str << NL;

    return 0;
}
```

To build with gcc/clang:
> g++ example.cpp
> clang++ example.cpp

Run:
> ./a.out
Hello Evo 123!

If the compiler can't find the Evo headers you will need to add an include path like the following.

Example with gcc/clang:
> g++ -I/opt/evo-0.4/include example.cpp
> clang++ -I/opt/evo-0.4/include example.cpp

For MS Visual C++ projects, add evo-0.4/include to "Include Directories" under: Project Properties -> VC++ Directories.

Compilers
---------

Systems:
 * Ubuntu LTS: 18.04, 16.04, 14.04
 * CentOS: 7, 6
 * FreeBSD: 11.1
 * MacOS: 10.13.6 (High Sierra)
 * Windows: 10, 7

Compilers:
 * GCC 7.3 - 4.4
 * Clang 6.0 - 3.4
 * Apple Xcode 10
 * Microsoft Visual Studio 2017 - 2013

For a full list of tested systems and compilers see the C++ Compilers section in the documentation.
