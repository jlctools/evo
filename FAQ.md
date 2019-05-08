Evo C++ Library FAQ
---------------

1. I have C++17 and various libraries for these things already -- why should I use Evo?
   - In addition to the performance benefits mentioned below, Evo provides a solid foundation of core classes and a lot of productivity features without bringing in additional dependencies
   - Portability with different compilers and older C++ standards
   - Consistency with different compilers -- for example: The `evo::String` class generally works the same in Windows and Linux/Unix
   - Exceptions are optional and may be disabled without losing functionality
   
2. What makes Evo "high performance" and unique here?
   - At a high level:
     - [Sharing](http://jlctools.github.io/evo/evo-0.5/html/Sharing.html) (Copy-On-Write) and [Slicing](http://jlctools.github.io/evo/evo-0.5/html/Slicing.html) (non-destructive editing) make complex string splitting, tokenizing, and parsing very fast.
       - Many implementations of std::string (especially in Linux) also used Copy-On-Write until C++11 [disallowed](https://stackoverflow.com/questions/12199710/legality-of-cow-stdstring-implementation-in-c11) it. Most compilers (like GCC 5+) have moved to Short-String-Optimization instead, which has it's own trade-offs. The "short" threshold is usually around 10-20 characters and depends on the compiler, and once exceeded the copy-optimization is lost and performance can quickly degrade.
     - Assigning a string literal (or other `char*`) to an `evo::String` doesn't allocate memory or make a copy -- Slicing is used here so the string simply references the literal storage. This is a helpful optimization, _but may lead to surprises_ when using `char*` pointers. For this reason, [Managed Pointers](http://jlctools.github.io/evo/evo-0.5/html/ManagedPtr.html) are preferred over raw pointers with code using Evo.
     - Exceptions are optional and may be conditionally disabled at runtime, when performance is critical
     - Evo strings have a lot of features for common operations (with performance in mind) with methods like `split()`, `token(),` `num()`, etc, where complex parsing can be done without copying string buffers or allocating memory. There are also tokenizer classes like `evo::StrTok` and `evo::StrTokQ`. See [String Parsing](http://jlctools.github.io/evo/evo-0.5/html/StringParsing.html). These features are especially useful in servers, which often do lots of string splitting and parsing.
   - There are some interface decisions that force callers to be explicit about using "mutable" methods -- for example evo::List `operator[]()` is const so can't modify the item. To modify an item at an index use the `operator()()` or `itemM()` method. This makes the caller more aware of which methods force a _copy_ with Copy-On-Write optimization.
   - Evo implements [number conversions](http://jlctools.github.io/evo/evo-0.5/html/StringConversion.html) that don't require terminated strings. This makes it efficient for converting sub-strings to numbers without having to store or search for a terminator.
   - Evo containers generally try to favor good cache locality (contiguous arrays) over defragmented memory like a linked-list, for example.
   - At a lower level some design decisions favor performance over safety, which may be considered controversial:
     - Containers require "relocatable" items, meaning an item must not store a pointer to itself or to a member variable. When a container (such as `evo::List`) grows and moves to a larger buffer, the safe thing to do is copy-construct each item to the new location. Evo containers instead use a single `memcpy()` to do this for best performance (avoiding a copy constructor call per item).
     - `evo::List` is relatively large and very complex due to the various sharing and slicing optimizations and the amount of features supported. It took months to thoroughly test and this is a core building block (`evo::String` inherits `evo::List<char>` for example). I felt the trade-off was worth the performance benefits and the details are encapsulated and tested.
     - Copy-On-Write optimization does not use atomics so synchronization is normally required when copying from one thread to another. This avoids any overhead from atomics and forces the user to be aware of the copy in a multithreaded application. For thread safety, synchronization is required and the container `copy()` or `unshare()` method should be called to force a copy. Another way to force a copy is to pass the string as `const evo::SubString&`, then assigning it to a `evo::String` forces a copy. See [String Passing](http://jlctools.github.io/evo/evo-0.5/html/StringPassing.html). This applies to other container types too.
   - SSE optimizations are used for low-level string operations not supported by built-in functions like `memchr()`, like skipping/finding whitespace -- SSE 2 and 4.2 are supported. GNU C extensions like `memrchr()` are also used when possible.
   - Other high-performance libraries often avoid using STL internally for similar reasons. Examples include [RapidJSON](http://rapidjson.org) and [Folly](https://github.com/facebook/folly). New libraries and applications can use Evo for these performance benefits.
   
4. Why doesn't Evo provide drop-in replacements for STL classes for familiar interfaces?
   
   - The short answer is Evo doesn't follow (and isn't limited by) the standard, and doesn't make the same guarantees
   - Evo classes are designed differently -- interface differences make this clear, explicit "mutable" methods being one example
   
4. What code style rules does Evo use?

   - Type names are `Capitalized` (`UpperCamelCase`)
   - Method and variable names use `snake_case`
   - Private member variable names have an underscore suffix to avoid naming conflicts, for example: `int foo_;`
   - Constants use `ALL_CAPS`
   - Namespaces are short and `lwrcase`
   - Some exceptions to explicitly differentiate certain features:
     - prefix using lower `camelCase` to differentiate methods for "advanced" or "intrusive" features, such as `advResize()` in `evo::List` and `evo::String` classes
     - capitalized suffix to differentiate:
       - "mutability" (vs const), such as `findM()` in `evo::Map`
       - "template" type variants, such as `evo::IntegerT`
     - enum value names are considered constants, but have a short `lowercase` prefix (to avoid naming conflicts) such as `tOBJECT` in `evo::Var::Type` (`t` for enum `Type`)

   



