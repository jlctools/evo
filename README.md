Evo C++ Library
---------------

_May 2019: Evo 0.5.1 Released!_

[Evo on Github](https://github.com/jlctools/evo)

* [Documentation](http://jlctools.github.io/evo/evo-0.5.1/html/index.html)
* [What's New](http://jlctools.github.io/evo/evo-0.5.1/html/_whats_new.html)
* [Benchmarks](bench/README.md)
* [FAQ](FAQ.md)

Evo is a modern high performance C++ library designed to make writing efficient and portable code easy. The core library offers alternatives to the standard library with many productivity and high performance features.

Inspired by modern languages like Python and D, Evo is the result of many years of lessons learned from writing (and rewriting) high-performance C++ server code. Evo aims to make C++ easier and more powerful without sacrificing performance.

Getting Started
---------------

See "*Features & Concepts*" and "*Mini Tutorials*" in the [documentation](http://jlctools.github.io/evo/evo-0.5.1/html/index.html), which include many examples. Try using some of the Evo classes on a project, or with a "hello world" example program, or play with the [benchmarks](bench/README.md). *This is the best way to quickly get a feel for working with Evo.*

Here's a basic "hello world" example, with some additions to make it more interesting:

```cpp
#include <evo/io.h>
#include <evo/string.h>
#include <evo/time.h>
using namespace evo;

int main() {
    // Get console streams
    Console& c = con();

    // Format a string with some numbers
    String text;
    text << "Testing " << 123 << ' ' << FmtUInt(0x1C8, fHEX);

    // Prompt for name and age
    String name, age;
    c.out << "Type your name: " << fFLUSH;
    c.in.readline(name);

    c.out << "Type your age: " << fFLUSH;
    c.in.readline(age);

    // Figure out date with birth year
    DateTime dt;
    dt.set_local().add_years(-age.num().value());

    // Write to stdout
    c.out << text << NL
        << "Hello World" << NL
        << "Hello " << name << '!' << NL
        << "You were born in " << dt.date.year << NL;

    return 0;
}
```

Output:

```
Type your name: John Smith
Type your age: 99
Testing 123 1C8
Hello World
Hello John Smith!
You were born in 1920
```



Download
--------

[Download Evo 0.5.1](https://github.com/jlctools/evo/tree/master/docs/downloads)

License
-------

Evo is Free and Open-Source Software licensed under the [2-Clause BSD License](https://opensource.org/licenses/BSD-2-Clause), which is included with Evo in the LICENSE.txt file.
