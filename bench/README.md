# Benchmarks

Results are from an Intel Core i7-6560U @ 2.20 GHz on Ubuntu 16.04 64-bit using GCC 5.4.0 (under Windows 10 Subsystem for Linux).

Compile with optimizations enabled:

```
$ g++ -O3 -std=c++11 -I.. -pthread -o string string.cpp && ./string
```

Or use the supplied `bench.sh` script (see `-h` for usage help):

```
$ ./bench.sh string
```

Benchmarks run test code repeatedly and measure time elapsed along with CPU time used.

* Each test runs enough times to get useful averages of elapsed time
* The `DiffBest` column shows the difference in average CPU used between the current test and the best (fastest) test in the set -- this will be `0` for the best test
* Lower time values are better

## Strings

String benchmarks compare Evo performance to STL and C equivalents for common string operations.

### Sizes

String sizes include `sizeof()` on the string type as well as the buffer header used for allocating memory. This doesn't include memory for storing the actual string. Here we compare Evo to STL.

```
Sizes:
 - evo str:        44: 32 + 12
 - evo substr:     16
 - stl str:        56: 32 + 24
 - C ptr + size_t: 16
```

### Splitting

These tests involve splitting an input string into two sub-strings, before and after a delimiter.

* Evo supports this with `split()` methods on `String` and `SubString` classes, which perform the best when  terminated sub-strings _aren't required_ -- both terminated and unterminated cases are covered
* With STL this is implemented using `std::string` `find()` and `assign()` methods
* With C this is implemented using: `strchr(), strncpy(), malloc(), free()`

The test runs with three variations of input strings: Long, Longer, and Short

**Results:**

* Both `evo::String` and `evo::SubString` outperform the alternatives, especially with non-terminated sub-strings
* Except `std::string` does perform the best with the "Short" case when terminated sub-strings are required (due to Short String Optimization)

SplitTestLong:
| Name                   | Time(nsec) | CPU(nsec)  | Count    | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ---------------------- | ---------- | ---------- | -------- | ------------- | ------------ | -------------- |
| BM::evo_String_Term    | 1693987800 | 1687500000 | 10000000 | 169.39878     | 168.75       | 135.9375       |
| BM::evo_SubString_Term | 1649859800 | 1640625000 | 10000000 | 164.98598     | 164.0625     | 131.25         |
| BM::evo_String         | 454130800  | 437500000  | 10000000 | 45.41308      | 43.75        | 10.9375        |
| BM::evo_SubString      | 329146400  | 328125000  | 10000000 | 32.91464      | 32.8125      | 0              |
| BM::stl                | 2254222800 | 2250000000 | 10000000 | 225.42228     | 225          | 192.1875       |
| BM::stl_Term           | 2241538500 | 2218750000 | 10000000 | 224.15385     | 221.875      | 189.0625       |
| BM::c                  | 668794000  | 656250000  | 10000000 | 66.8794       | 65.625       | 32.8125        |

SplitTestLonger:
| Name                   | Time(nsec) | CPU(nsec)  | Count    | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ---------------------- | ---------- | ---------- | -------- | ------------- | ------------ | -------------- |
| BM::evo_String_Term    | 2032568100 | 2031250000 | 10000000 | 203.25681     | 203.125      | 142.1875       |
| BM::evo_SubString_Term | 2029138300 | 2015625000 | 10000000 | 202.91383     | 201.5625     | 140.625        |
| BM::evo_String         | 836978100  | 828125000  | 10000000 | 83.69781      | 82.8125      | 21.875         |
| BM::evo_SubString      | 627551200  | 609375000  | 10000000 | 62.75512      | 60.9375      | 0              |
| BM::stl                | 2410352900 | 2406250000 | 10000000 | 241.03529     | 240.625      | 179.6875       |
| BM::stl_Term           | 2488852500 | 2484375000 | 10000000 | 248.88525     | 248.4375     | 187.5          |
| BM::c                  | 748672300  | 734375000  | 10000000 | 74.86723      | 73.4375      | 12.5           |

SplitTestShort:
| Name                   | Time(nsec) | CPU(nsec)  | Count    | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ---------------------- | ---------- | ---------- | -------- | ------------- | ------------ | -------------- |
| BM::evo_String_Term    | 1500870200 | 1484375000 | 10000000 | 150.08702     | 148.4375     | 132.8125       |
| BM::evo_SubString_Term | 1455932100 | 1453125000 | 10000000 | 145.59321     | 145.3125     | 129.6875       |
| BM::evo_String         | 336292800  | 328125000  | 10000000 | 33.62928      | 32.8125      | 17.1875        |
| BM::evo_SubString      | 165987000  | 156250000  | 10000000 | 16.5987       | 15.625       | 0              |
| BM::stl                | 533767900  | 515625000  | 10000000 | 53.37679      | 51.5625      | 35.9375        |
| BM::stl_Term           | 643139200  | 640625000  | 10000000 | 64.31392      | 64.0625      | 48.4375        |
| BM::c                  | 658200200  | 656250000  | 10000000 | 65.82002      | 65.625       | 50             |



### Tokenizing  to Numbers

This test tokenizes a string of comma-separated numbers into actual number values.

* Evo supports this with `evo::StrTok` and `evo::SubString` number conversion methods like `getnum()` -- these work together without allocating memory or copying sub-strings
* With STL this is implemented using `std::string` `find()` and `assign()` methods, and `strtoul()` for number conversion
* With C this is implemented using: `memcpy(), strtoul(), malloc(), free()`

**Results:**

* Evo outperforms the alternatives by far here

TokNum:
| Name    | Time(nsec) | CPU(nsec) | Count  | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------ | ------------- | ------------ | -------------- |
| BM::evo | 41593100   | 31250000  | 100000 | 415.931       | 312.5        | 0              |
| BM::stl | 97157500   | 93750000  | 100000 | 971.575       | 937.5        | 625            |
| BM::c   | 144472100  | 140625000 | 100000 | 1444.721      | 1406.25      | 1093.75        |

### Tokenizing to Sub-Strings

These tests tokenize a string of comma-separated values into sub-strings.

* Evo supports this with `evo::StrTok` (similar to the above TokNum test), _but for a more fair comparison the sub-strings are also converted to terminated strings_
* With STL this is implemented using `std::string` `find()` and `assign()` methods
* With C this is implemented using: `memcpy(), malloc(), free()`

The test runs with 5 different input variants to cover different scenarios and edge cases.

**Results:**

* Evo outperforms the alternatives in all cases here, _even while making sure all sub-strings are terminated_

TokStr1:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 298249300  | 296875000 | 1000000 | 298.2493      | 296.875      | 0              |
| BM::stl | 314294000  | 312500000 | 1000000 | 314.294       | 312.5        | 15.625         |
| BM::c   | 312994300  | 312500000 | 1000000 | 312.9943      | 312.5        | 15.625         |

TokStr2:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 241041800  | 234375000 | 1000000 | 241.0418      | 234.375      | 0              |
| BM::stl | 346329300  | 343750000 | 1000000 | 346.3293      | 343.75       | 109.375        |
| BM::c   | 291613800  | 281250000 | 1000000 | 291.6138      | 281.25       | 46.875         |

TokStr3:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 252414500  | 250000000 | 1000000 | 252.4145      | 250          | 0              |
| BM::stl | 329709500  | 328125000 | 1000000 | 329.7095      | 328.125      | 78.125         |
| BM::c   | 308602700  | 296875000 | 1000000 | 308.6027      | 296.875      | 46.875         |

TokStr4:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 284557300  | 296875000 | 1000000 | 284.5573      | 296.875      | 0              |
| BM::stl | 393056100  | 375000000 | 1000000 | 393.0561      | 375          | 78.125         |
| BM::c   | 352435800  | 359375000 | 1000000 | 352.4358      | 359.375      | 62.5           |

TokStr5:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 96507100   | 93750000  | 1000000 | 96.5071       | 93.75        | 0              |
| BM::stl | 190957000  | 171875000 | 1000000 | 190.957       | 171.875      | 78.125         |
| BM::c   | 156129900  | 156250000 | 1000000 | 156.1299      | 156.25       | 62.5           |

### Formatting

This test formats a string with some numbers and a sub-string then does a string comparison to verify the result.

* Evo supports this with `evo::String` `operator<<()`  and `operator!=()` for comparison
* With STL this is implemented with `std::ostringstream`, which is then converted to `std::string` and compared with `operator!=()`
* With C this is implemented with `snprintf()` to a hard-coded buffer and compared with `strcmp()`

**Results:**

* Evo outperforms both alternatives here, and the STL approach with `std::ostringstream` is especially weak here

StrFmt:
| Name    | Time(nsec) | CPU(nsec)  | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | ---------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 281661300  | 281250000  | 1000000 | 281.6613      | 281.25       | 0              |
| BM::stl | 1030297400 | 1015625000 | 1000000 | 1030.2974     | 1015.625     | 734.375        |
| BM::c   | 374214000  | 375000000  | 1000000 | 374.214       | 375          | 93.75          |

### MSVC 2017 (15.9.11)

Results are similar with these tests under MSVC, though there are some cases where MSVC STL outperforms Evo. Results for these cases are shown below. This is due to Evo making terminated sub-strings and MSVC STL using Short String Optimization. _If these tests are modified so sub-strings aren't terminated then Evo outperforms STL here (not shown)_.

TokStr1:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 356658200  | 343750000 | 1000000 | 356.6582      | 343.75       | 31.25          |
| BM::stl | 320038000  | 312500000 | 1000000 | 320.038       | 312.5        | 0              |
| BM::c   | 428172500  | 406250000 | 1000000 | 428.1725      | 406.25       | 93.75          |

TokStr2:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 385779700  | 390625000 | 1000000 | 385.7797      | 390.625      | 31.25          |
| BM::stl | 367974800  | 359375000 | 1000000 | 367.9748      | 359.375      | 0              |
| BM::c   | 462347500  | 468750000 | 1000000 | 462.3475      | 468.75       | 109.375        |

TokStr3:
| Name    | Time(nsec) | CPU(nsec) | Count   | AvgTime(nsec) | AvgCPU(nsec) | DiffBest(nsec) |
| ------- | ---------- | --------- | ------- | ------------- | ------------ | -------------- |
| BM::evo | 335343800  | 343750000 | 1000000 | 335.3438      | 343.75       | 15.625         |
| BM::stl | 340966000  | 328125000 | 1000000 | 340.966       | 328.125      | 0              |
| BM::c   | 439441000  | 437500000 | 1000000 | 439.441       | 437.5        | 109.375        |