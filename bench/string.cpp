// Evo C++ Library
///////////////////////////////////////////////////////////////////////////////

#include <evo/benchmark.h>
#include <evo/string.h>
#include <evo/strtok.h>
#include <string>
#include <sstream>
#if defined(EVO_CPP17)
    #include <string_view>
#endif
using namespace evo;

// Output types: tTEXT or tMARKDOWN
#if !defined(EVO_BENCH_OUTPUT_TYPE)
    #define EVO_BENCH_OUTPUT_TYPE tMARKDOWN
#endif

static const FmtTable::Type fmt_type = FmtTable::EVO_BENCH_OUTPUT_TYPE;

#if _MSC_VER
    #define strtok_r strtok_s
    #pragma warning(push)
    #pragma warning(disable:4996)
#endif

struct SplitTest {
    struct InputShort {
        static const char* get_input() {
            return "Key=Value";
        }
    };
    struct InputLong {
        static const char* get_input() {
            return "Key_no_short_string_optimization=Value_no_short_string_optimization";
        }
    };
    struct InputLonger {
        static const char* get_input() {
            return "Key_no_short_string_optimization_no_short_string_optimization_no_short_string_optimization=Value_no_short_string_optimization_no_short_string_optimization_no_short_string_optimization";
        }
    };

    template<class T> struct BM {
        static const char* get_input() {
            // Copy input to static String so compiler doesn't optimize out code being benchmarked
            static const char* p = T::get_input();
            static const String str(p, (StrSizeT)strlen(p));
            return str.data();
        }

        static void evo_String_Term() {
            String in(get_input()), key, val;
            if (!in.split('=', key, val))
                abort();
            // Make terminated
            key.cstr();
            val.cstr();
        }

        static void evo_SubString_Term() {
            SubString in(get_input()), key, val;
            if (!in.split('=', key, val))
                abort();
            // Make terminated
            String buf1, buf2;
            key.cstr(buf1);
            val.cstr(buf2);
        }

        static void evo_String() {
            String in(get_input()), key, val;
            if (!in.split('=', key, val))
                abort();
        }

        static void evo_SubString() {
            SubString in(get_input()), key, val;
            if (!in.split('=', key, val))
                abort();
        }

        static void stl() {
            std::string in(get_input()), key, val;
            size_t pos = in.find('=');
            if (pos == std::string::npos)
                abort();
            key.assign(in.data(), pos);
            val.assign(in.data()+pos+1, in.length()-pos-1);
        }

        static void stl_Term() {
            std::string in(get_input()), key, val;
            size_t pos = in.find('=');
            if (pos == std::string::npos)
                abort();
            key.assign(in.data(), pos);
            val.assign(in.data()+pos+1, in.length()-pos-1);
            // Make terminated
            key.c_str();
            val.c_str();
        }

        static void c() {
            const char* in;
            const char* val;
            char* key;
            size_t key_len;

            in = get_input();
            val = strchr(in, '=');
            if (val == NULL)
                abort();

            key_len = (size_t)(val - in);
            key = (char*)malloc(key_len+1);
            strncpy(key, in, key_len);
            key[key_len] = '\0';
            ++val;
            free(key);
        }
    };
};

struct TokNumTest {
    static const char* get_input() {
        // Copy input to static String so compiler doesn't optimize out code being benchmarked
        const char* STR = "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30";
        static String str(STR, (StrSizeT)strlen(STR));
        return str.cstr();
    }

    static void evo() {
        StrTok tok(get_input());
        for (ulong j=1; tok.next(','); ++j)
            if (tok.value().getnum<ulong>(10) != j)
                abort();
    }

    static void stl() {
        std::string in(get_input()), tok;
        size_t pos = 0, next_pos;
        for (ulong j=1; pos != std::string::npos; ++j) {
            next_pos = in.find(',', pos);
            if (next_pos == std::string::npos) {
                tok.assign(in.data()+pos, in.length()-pos);
                pos = std::string::npos;
            } else {
                tok.assign(in.data()+pos, next_pos-pos);
                pos = next_pos + 1;
            }
            if (strtoul(tok.c_str(), NULL, 10) != j)
                abort();
        }
    }

    static void c() {
        const char* tok;
        char* save_ptr = NULL;

        const char* in_str = get_input();
        size_t in_len = strlen(in_str);
        char* in      = (char*)malloc(in_len+1);
        memcpy(in, in_str, in_len+1);

        for (ulong j=1;; ++j) {
            tok = strtok_r((j==1?in:NULL), ",", &save_ptr);
            if (tok == NULL)
                break;
            if (strtoul(tok, NULL, 10) != j)
                abort();
        }

        free(in);
    }
};

struct TokStrTest {
    struct Input1 {
        static const char* get_input() {
            return "one,2,three,four is bigger,five,six,seven is last and much bigger than four is now";
        }
    };
    struct Input2 {
        static const char* get_input() {
            return "one is first and this time is must larger than the others,2,three,four is bigger,five,six,seven";
        }
    };
    struct Input3 {
        static const char* get_input() {
            return "one,2,three,four is in the middle and is this time the largest here now,five,six,seven";
        }
    };
    struct Input4 {
        static const char* get_input() {
            return "one_no_short_string_optimization,two_no_short_string_optimization,three_no_short_string_optimization,four_no_short_string_optimization,five_no_short_string_optimization,six_no_short_string_optimization,seven_no_short_string_optimization";
        }
    };
    struct Input5 {
        static const char* get_input() {
            return "one big item is the largest and only item here so no delimiters here to find at the moment";
        }
    };

    template<class T> struct BM {
        static const char* get_input() {
            // Copy input to static String so compiler doesn't optimize out code being benchmarked
            static const char* p = T::get_input();
            static const String str(p, (StrSizeT)strlen(p));
            return str.data();
        }

        static void evo() {
            String tmp;
            StrTok tok(get_input());
            while (tok.next(','))
                tok.value().cstr(tmp);
        }

        static void stl() {
            std::string tmp, inp(get_input());
            size_t pos = 0, next_pos;
            while (pos != std::string::npos) {
                next_pos = inp.find(',', pos);
                if (next_pos == std::string::npos) {
                    tmp.assign(inp.data()+pos, inp.length()-pos);
                    pos = std::string::npos;
                } else {
                    tmp.assign(inp.data()+pos, next_pos-pos);
                    pos = next_pos + 1;
                }
                tmp.c_str();
            }
        }

        static void c() {
            const char* in_str = get_input();
            const char* tmp;
            char* save_ptr = NULL;

            size_t in_len = strlen(in_str);
            char* inp     = (char*)malloc(in_len+1);
            memcpy(inp, in_str, in_len+1);

            uint j = 0;
            do {
                tmp = strtok_r((j==0 ? inp : NULL), ",", &save_ptr);
                ++j;
            } while (tmp != NULL);
            free(inp);
        }
    };
};

struct StrFmtTest {
    static void evo() {
        String str;
        str << "This is a test " << 123 << ' ' << 4.56 << " and here's a " << "suffix";
        if (str != "This is a test 123 4.56 and here's a suffix")
            abort();
    }

    static void stl() {
        std::ostringstream out;
        out << "This is a test " << 123 << ' ' << 4.56 << " and here's a " << "suffix";
        if (out.str() != "This is a test 123 4.56 and here's a suffix")
            abort();
    }

    static void c() {
        const int BUF_SIZE = 64;
        char str[BUF_SIZE];
        snprintf(str, BUF_SIZE, "This is a test %i %g and here's a %s", 123, 4.56, "suffix");
        if (strcmp(str, "This is a test 123 4.56 and here's a suffix") != 0)
            abort();
    }
};

// Helper for getting string size info
struct StrSz {
    struct EvoStr : public String
        { static const int hdr_sz = sizeof(String::Header); }; // use inheritance to access protected member
    static const int evo_str_sz   = sizeof(String);
    static const int evo_hdr_sz   = EvoStr::hdr_sz;
    static const int evo_sz       = evo_str_sz + evo_hdr_sz;

    static const int stl_str_sz = sizeof(std::string);
#if defined(EVO_GLIBCPP)
    // GCC 4.9, 5.4, 7.3 -- may not be accurate for others
    struct _Rep_base {
        std::string::size_type _M_length;
        std::string::size_type _M_capacity;
        _Atomic_word           _M_refcount;
    };
    static const int  stl_hdr_sz     = sizeof(_Rep_base);
    static const bool stl_hdr_sz_unk = false;
#elif defined(_MSC_VER)
    static const int  stl_hdr_sz     = sizeof(std::string::_Mydata_t);
    static const bool stl_hdr_sz_unk = false;
#else
    static const int  stl_hdr_sz     = 0;
    static const bool stl_hdr_sz_unk = true;
#endif
    static const int stl_sz = stl_str_sz + stl_hdr_sz;

    static String fmt_hdr_size() {
        String str;
        if (stl_hdr_sz_unk)
            str << '?';
        else
            str << stl_hdr_sz;
        return str;
    }
};

int main() {
    Console& c = con();

    c.out << "Config:" << NL
        << " - Compiler              " << EVO_COMPILER << ' ' << EVO_COMPILER_VER << NL
        << " - EVO_LIST_REALLOC      " << EVO_LIST_REALLOC << NL
        //TODO << " - EVO_ALLOCATORS        " << EVO_ALLOCATORS << NL
        //TODO << " - EVO_LIST_OPT_REFTERM  " << EVO_LIST_OPT_REFTERM << NL
        ;

    c.out << "Sizes:" << NL
        << " - evo str:        " << StrSz::evo_sz << ": " << StrSz::evo_str_sz << " + " << StrSz::evo_hdr_sz << NL
        << " - evo substr:     " << sizeof(SubString) << NL
        << " - stl str:        " << StrSz::stl_sz << ": " << StrSz::stl_str_sz << " + " << StrSz::fmt_hdr_size() << NL
    #if defined(EVO_CPP17)
        << " - stl strview:    " << sizeof(std::string_view) << NL
    #endif
        << " - C ptr + size_t: " << sizeof(void*) + sizeof(size_t) << NL
    ;

    c.out << NL;


    #define RUN_SPLIT_TEST(T) { \
        typedef SplitTest::BM<T> BM; \
        EVO_BENCH_SETUP(BM::c, 1000); \
        EVO_BENCH_RUN(BM::evo_String_Term); \
        EVO_BENCH_RUN(BM::evo_SubString_Term); \
        EVO_BENCH_RUN(BM::evo_String); \
        EVO_BENCH_RUN(BM::evo_SubString); \
        EVO_BENCH_RUN(BM::stl); \
        EVO_BENCH_RUN(BM::stl_Term); \
        EVO_BENCH_RUN(BM::c); \
        bench.report(fmt_type); \
    }

    c.out << "SplitTestLong:" << NL;
    RUN_SPLIT_TEST(SplitTest::InputLong);

    c.out << "SplitTestLonger:" << NL;
    RUN_SPLIT_TEST(SplitTest::InputLonger);

    c.out << "SplitTestShort:" << NL;
    RUN_SPLIT_TEST(SplitTest::InputShort);


    #define RUN_TEST(T) { \
        typedef T BM; \
        EVO_BENCH_SETUP(BM::c, 1000); \
        EVO_BENCH_RUN(BM::evo); \
        EVO_BENCH_RUN(BM::stl); \
        EVO_BENCH_RUN(BM::c); \
        bench.report(fmt_type); \
    }

    c.out << "TokNum:" << NL;
    RUN_TEST(TokNumTest);


    #define RUN_TOK_STR_TEST(T) RUN_TEST(TokStrTest::BM<T>)

    c.out << "TokStr1:" << NL;
    RUN_TOK_STR_TEST(TokStrTest::Input1);

    c.out << "TokStr2" << NL;
    RUN_TOK_STR_TEST(TokStrTest::Input2);

    c.out << "TokStr3:" << NL;
    RUN_TOK_STR_TEST(TokStrTest::Input3);

    c.out << "TokStr4:" << NL;
    RUN_TOK_STR_TEST(TokStrTest::Input4);

    c.out << "TokStr5:" << NL;
    RUN_TOK_STR_TEST(TokStrTest::Input5);

    c.out << "StrFmt:" << NL;
    RUN_TEST(StrFmtTest);

    return 0;
}

#if _MSC_VER
    #pragma warning(pop)
#endif
