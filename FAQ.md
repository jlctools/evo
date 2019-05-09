Evo C++ Library FAQ
---------------

1. Why?
2. I have C++17 and various libraries for these things already -- why should I use Evo?
3. What makes Evo "high performance" and unique here?
4. Evo supports C++03, so why is it "modern"?
5. Why replicate standard library types like pair?
6. Why not build on top of STL (like Boost)?
7. Why doesn't Evo provide drop-in replacements for STL classes for familiar interfaces?
8. What code style rules does Evo use?

## Answers

1. Why?
   
   * Evo started in the mid 2000's with `SubString` -- a simple class that allowed much faster string processing. Next, this led to `String` with Copy-On-Write optimization and it grew from there. I released Evo as Open Source around 2015 so others could use it and so I could use it at work.
   
     I've implemented classes like these many times at work on various projects. They did the job, but I thought they could be better with some more thought so I experimented on my own time. At the time I was working on Solaris, where memory allocations were relatively expensive, and just minimizing memory allocations resulted in large performance gains on production servers. After that I found the next performance bottleneck was often from copying data around, and Copy-On-Write and Slicing optimizations resulted in more performance gains.
   
     This leads to a common performance hurdle with C and C++: _terminated strings_
   
     Even `std::string` still uses terminated strings, which is why  the `c_str()` method can be `const`. Evo avoids this assumption that strings must always be terminated, and so the Evo String `cstr()` method is _not_ `const` (unless you pass it a buffer to use).
   
     A `String` is really a `List` (i.e. vector) of characters, and so implementing `String` led to implementing the fundamental `List` class (dynamic array). With `String` there is also string formatting: Evo has formatting support for `String` that works the same as `Stream` types (`File`, `Socket`, etc) using `operator<<()`. This works much better than `std::ostringstream`, which is very slow by comparison.
   
     Evo has grown in this way from the viewpoint of writing high performance server code. Additionally I've used [Python](https://www.python.org/) a lot for various things and it makes a lot of common programming tasks easy, but which are tricky and error-prone with C++. Basic string processing is a good example here -- things like splitting a string into tokens and sub-tokens and converting some parts to numbers, or handling command-line arguments. The thinking here is that these very basic tasks should not require building or searching for another 3rd party library -- though of course you could still mix other libraries with Evo, if needed.
   
2. I have C++17 and various libraries for these things already -- why should I use Evo?

   - In addition to the performance benefits mentioned below, Evo provides a solid foundation of core classes and a lot of productivity features without bringing in additional dependencies
   - Portability with different compilers and older C++ standards
   - Consistency with different compilers -- for example: The `evo::String` class generally works the same in Windows and Linux/Unix
   - Exceptions are optional and may be disabled without losing functionality
   - Evo plays nice with STL and other libraries -- only use what you need

3. What makes Evo "high performance" and unique here?

   - At a high level:
     - [Sharing](http://jlctools.github.io/evo/evo-0.5.1/html/_sharing.html) (Copy-On-Write) and [Slicing](http://jlctools.github.io/evo/evo-0.5.1/html/_slicing.html) (non-destructive editing) make complex string splitting, tokenizing, and parsing very fast.
       - Many implementations of `std::string` (especially in Linux) also used Copy-On-Write until C++11 [disallowed](https://stackoverflow.com/questions/12199710/legality-of-cow-stdstring-implementation-in-c11) it. Most compilers (like GCC 5+) have moved to Short-String-Optimization instead, which has it's own trade-offs. The "short" threshold is usually around 10-20 characters and depends on the compiler, and once exceeded the copy-optimization is lost and performance can quickly degrade.
     - Assigning a string literal (or other `char*`) to an `evo::String` doesn't allocate memory or make a copy -- Slicing is used here so the string simply references the literal storage. This is a helpful optimization, _but may lead to surprises_ when using `char*` pointers. For this reason, [Managed Pointers](http://jlctools.github.io/evo/evo-0.5.1/html/_managed_ptr.html) are preferred over raw pointers with code using Evo.
     - Exceptions are optional and may be conditionally disabled at runtime, when performance is critical
     - Evo strings have a lot of features for common operations (with performance in mind) with methods like `split()`, `token(),` `num()`, etc, where complex parsing can be done without copying string buffers or allocating memory. There are also tokenizer classes like `evo::StrTok` and `evo::StrTokQ`. See [String Parsing](http://jlctools.github.io/evo/evo-0.5.1/html/_string_parsing.html). These features are especially useful in servers, which often do lots of string splitting and parsing.
   - There are some interface decisions that force callers to be explicit about using "mutable" methods -- for example evo::List `operator[]()` is `const` so can't modify the item. To modify an item at an index use the `operator()()` or `itemM()` method. This makes the caller more aware of which methods force a _copy_ with Copy-On-Write optimization.
   - Evo implements [number conversions](http://jlctools.github.io/evo/evo-0.5.1/html/_string_conversion.html) that don't require terminated strings. This makes it efficient for converting sub-strings to numbers without having to store or search for a terminator.
   - Evo containers generally try to favor good cache locality (contiguous arrays) over defragmented memory like a linked-list, for example.
   - At a lower level some design decisions favor performance over safety, which may be considered controversial:
     - Containers require "relocatable" items, meaning an item must not store a pointer to itself or to a member variable. When a container (such as `evo::List`) grows and moves to a larger buffer, the safe thing to do is copy-construct each item to the new location. Evo containers instead use a single `memcpy()` to do this for best performance (avoiding a copy constructor call per item).
     - `evo::List` is relatively large and very complex due to the various sharing and slicing optimizations and the amount of features supported. It took a lot of work to thoroughly test and this is a core building block (`evo::String` inherits `evo::List<char>` for example). I felt the trade-off was worth the performance benefits and the details are encapsulated and tested.
     - Copy-On-Write optimization does not use atomics so synchronization is normally required when copying from one thread to another. This avoids any overhead from atomics and forces the user to be aware of the copy in a multithreaded application. For thread safety, synchronization is required and the container `copy()` or `unshare()` method should be called to force a copy. Another way to force a copy is to pass the string as `const evo::SubString&`, then assigning it to a `evo::String` forces a copy. See [String Passing](http://jlctools.github.io/evo/evo-0.5.1/html/_string_passing.html). This applies to other container types too.
   - SSE optimizations are used for low-level string operations not supported by built-in functions like `memchr()`, like skipping/finding whitespace -- SSE 2 and 4.2 are supported. GNU C extensions like `memrchr()` are also used when possible.
   - Other high-performance libraries often avoid using STL internally for similar reasons. Examples include [RapidJSON](http://rapidjson.org) and [Folly](https://github.com/facebook/folly). New libraries and applications can use Evo for these performance benefits.

4. Evo supports C++03, so why is it "modern"?

   * Evo takes a lot of inspiration from modern languages like Python and D, and it makes C++ start to feel more like these languages due to all the basic productivity features. You can still use more modern design and thinking with C++03.
   * Evo also embraces the newer standards where this makes sense -- for example, Evo containers support "initialization lists" with C++11

5. Why replicate standard library types like pair?

   * `evo::Pair` is mainly used by `evo::Map` types and has methods for different contexts to make code more readable, for example:

     ```cpp
     MapList<int,int> map;
     for (auto& item : map.asconst()) {
         item.key();
         item.value();
     }
     ```

     

6. Why not build on top of STL, like Boost?

   * Evo has [STL compatibility](http://jlctools.github.io/evo/evo-0.5.1/html/_stl_compatibility.html), so you can use what you want from both. For example, you can pass an `std::string` to functions accepting `const evo::String&` or `const evo::SubString&`, or to tokenizers like `evo::StrTok`.
   * Why Evo itself doesn't use STL containers:
     * Evo uses fundamentally different container and iterator implementations, and usually can outperform the STL -- see [Benchmarks](bench/README.md) for some examples
     * STL has issues with performance-critical use, like with servers and game engines -- see [EASTL ](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2271.html) for some examples
     * STL containers generally don't support "intrusive" use, such as writing directly to internal container buffers

7. Why doesn't Evo provide drop-in replacements for STL classes for familiar interfaces?

   - The short answer is Evo doesn't follow (and isn't limited by) the standard, and doesn't make the same guarantees
   - Evo classes are designed differently -- interface differences make this clear, explicit "mutable" methods being one example

8. What code style rules does Evo use?

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
   - Structured Naming:
     - This is a naming scheme where related things are named so that they would sort next to each other -- the words in a name forms a sort of hierarchy with the most general components first
     - Evo uses this for naming things like `MapList` and `MapHash` (instead of ListMap and HashMap), and with method variants with different suffixes like: `item(), itemM(), split(), and splitr()`. Advanced (intrusive) methods have their own common category with the `adv` prefix, for example: `advItem()`
     - This is the rationale for some naming that may look "odd" or backwards at first


