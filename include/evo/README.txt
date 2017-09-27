Evo C++ Library
---------------
Version: 0.3

Evo is a modern high-performance C++ library designed to make handling data structures and
strings easy.

Evo is Free and Open-Source Software licensed under Mozilla Public License 2.0.

 * Evo can be used in any software and can be distributed under the terms of the MPL
 * MPL does not automatically apply to your programs using Evo, it only applies to Evo files
 * A copy of the MPL 2.0 should have been included with Evo (LICENSE-MPL2.txt)
 * For more information see the MPL FAQ: https://www.mozilla.org/MPL/2.0/FAQ.html

For the latest version of Evo: http://jlctools.us/evo

Using Evo
---------

You may include all core Evo headers like the following or include individual headers as needed.
Note that Evo headers are best included in the form #include <evo/HEADER.h> (from the parent
directory). See Evo documentation for more detail.

// Include all core Evo headers
#include <evo/core.h>

A quick example.cpp to get you started:
#include <evo/core.h>

int main() {
    evo::String str;
    str << "Hello Evo " << 123 << '!';
    printf("%s\n", str.cstr());
    return 0;
}

To build with gcc:
> g++ example.cpp

Run:
> ./a.out
Hello Evo 123!

If the compiler can't find the Evo headers you will need to add an include path like the following.

Example with gcc:
> g++ -I/opt/evo0/include example.cpp

Compilers
---------

Evo has extensive unit test coverage and is tested under:

 * Ubuntu 64-bit
   * 14.04, gcc 4.8.4, valgrind 3.10.1
   * 12.04, gcc 4.6.3, valgrind 3.7.0
 * CentOS 64-bit
   * 7.1, gcc 4.8.5, valgrind 3.10.0
   * 6.7, gcc 4.4.7, valgrind 3.8.1
     * gcc bug: Tests crash ("pure virtual function call") on release build with optimization level above -O1
   * 5.7 64-bit, gcc 4.1.2, valgrind 3.5.0
     * Note: Release build may give warnings due to older compiler
 * Windows 7 32-bit:
   * cygwin gcc 4.9.2
   * MSVC 2010 Express SP1
