// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file strscan.h Evo string scanning helpers with SSE optimized code. */
#pragma once
#ifndef INCL_evo_impl_strscan_h
#define INCL_evo_impl_strscan_h

#include "type.h"
#include "impl/str.h"
#include <string.h>

#if defined(EVO_CPU)
    #if defined(_WIN32)
        #include <intrin.h>
        #if !defined(EVO_IMPL_SSE42) && defined(EVO_IMPL_SSE2)
            #pragma intrinsic(_BitScanForward)
            #pragma intrinsic(_BitScanReverse)
        #endif
    #elif defined(EVO_IMPL_SSE42)
        #include <nmmintrin.h>
    #elif defined(EVO_IMPL_SSE2)
        #include <emmintrin.h>
    #endif
#endif

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl {
    // Scan to next non-whitespace char
    inline const char* str_scan_nws_default(const char* str, const char* end) {
        for (; str < end; ++str)
            if (!(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r'))
                break;
        return str;
    }

    // Scan to next non-whitespace char in reverse, return new end
    inline const char* str_scan_nws_default_r(const char* str, const char* end) {
        while (str < end) {
            --end;
            if (!(*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
                return end + 1;
        }
        return end;
    }

    // Scan to next non-delim char
    inline const char* str_scan_ndelim_default(const char* str, const char* end, char delim1, char delim2) {
        for (; str < end; ++str)
            if (!(*str == delim1 || *str == delim2))
                break;
        return str;
    }

    // Scan to next non-delim char in reverse, return new end
    inline const char* str_scan_ndelim_default_r(const char* str, const char* end, char delim1, char delim2) {
        while (str < end) {
            --end;
            if (!(*end == delim1 || *end == delim2))
                return end + 1;
        }
        return end;
    }

    // Scan to next delim
    inline const char* str_scan_delim_default(const char* str, const char* end, char delim1, char delim2) {
        for (; str < end; ++str)
            if (*str == delim1 || *str == delim2)
                break;
        return str;
    }

    // Scan to next delim in reverse, return new end
    inline const char* str_scan_delim_default_r(const char* str, const char* end, char delim1, char delim2) {
        while (str < end) {
            --end;
            if (*end == delim1 || *end == delim2)
                return end + 1;
        }
        return end;
    }

    // Scan to next delim
    inline const char* str_scan_delim_default(const char* str, const char* end, const char* delims, uint delim_count) {
        for (; str < end; ++str)
            if (::memchr(delims, *str, delim_count) != NULL)
                break;
        return str;
    }

    // Scan to next delim in reverse, return new end
    inline const char* str_scan_delim_default_r(const char* str, const char* end, const char* delims, uint delim_count) {
        while (str < end) {
            --end;
            if (::memchr(delims, *end, delim_count) != NULL)
                return end + 1;
        }
        return end;
    }

#if defined(EVO_CPU)
    static const size_t SSE_BATCH_SIZE = 16;
    static const size_t SSE_ALIGN16    = 0x0F;
    static const size_t SSE_ALIGN16_MASK = ~SSE_ALIGN16;
#endif

#if defined(EVO_IMPL_SSE42)
    // Scan to next non-whitespace char
    inline const char* str_scan_nws_cpu(const char* str, const char* end) {
        if (str < end) {
            // Scan up to alignment boundary
            const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            while (str < align16)
                if (!(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') || ++str == end)
                    return str;

            // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
            const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT | _SIDD_NEGATIVE_POLARITY;
            const __m128i ws = _mm_loadu_si128((__m128i*)" \t\n\r\0\0\0\0\0\0\0\0\0\0\0");
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            for (; str < align16; str += SSE_BATCH_SIZE) {
                const int i = _mm_cmpistri(ws, _mm_load_si128((__m128i*)str), FLAGS);
                if (i != (int)SSE_BATCH_SIZE)
                    return str + i;
            }

            // Scan remaining chars
            for (; str < end; ++str)
                if (!(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r'))
                    break;
        }
        return str;
    }

    // Scan to next non-whitespace char in reverse, return new end
    inline const char* str_scan_nws_cpu_r(const char* str, const char* end) {
        if (str < end) {
            const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (align16 > str) {
                // Scan up to alignment boundary
                while (end > align16) {
                    --end;
                    if (!(*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
                        return end + 1;
                }

                // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
                const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_MOST_SIGNIFICANT  | _SIDD_NEGATIVE_POLARITY;
                const __m128i ws = _mm_loadu_si128((__m128i*)" \t\n\r\0\0\0\0\0\0\0\0\0\0\0");
                align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
                while (end > align16) {
                    end -= SSE_BATCH_SIZE;
                    const int i = _mm_cmpistri(ws, _mm_load_si128((__m128i*)end), FLAGS);
                    if (i != (int)SSE_BATCH_SIZE)
                        return end + i + 1;
                }
            }

            // Scan remaining chars
            while (str < end) {
                --end;
                if (!(*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
                    return end + 1;
            }
        }
        return end;
    }

    ////

    // Scan to next non-delim char
    inline const char* str_scan_ndelim_cpu(const char* str, const char* end, char delim1, char delim2) {
        if (str < end) {
            // Scan up to alignment boundary
            const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            while (str < align16)
                if (!(*str == delim1 || *str == delim2) || ++str == end)
                    return str;

            // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (str < align16) {
                __m128i delims_in;
                {
                    char buf[SSE_BATCH_SIZE];
                    buf[0] = delim1;
                    ::memset(buf + 1, delim2, SSE_BATCH_SIZE - 1);
                    delims_in = _mm_loadu_si128((__m128i*)buf);
                }

                const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT | _SIDD_NEGATIVE_POLARITY;
                do {
                    const int i = _mm_cmpistri(delims_in, _mm_load_si128((__m128i*)str), FLAGS);
                    if (i != (int)SSE_BATCH_SIZE)
                        return str + i;
                    str += SSE_BATCH_SIZE;
                } while (str < align16);
            }

            // Scan remaining chars
            for (; str < end; ++str)
                if (!(*str == delim1 || *str == delim2))
                    break;
        }
        return str;
    }

    // Scan to next non-delim char in reverse, return new end
    inline const char* str_scan_ndelim_cpu_r(const char* str, const char* end, char delim1, char delim2) {
        if (str < end) {
            const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (align16 > str) {
                // Scan up to alignment boundary
                while (end > align16) {
                    --end;
                    if (!(*end == delim1 || *end == delim2))
                        return end + 1;
                }

                // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
                align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
                if (end > align16) {
                    __m128i delims_in;
                    {
                        char buf[SSE_BATCH_SIZE];
                        buf[0] = delim1;
                        ::memset(buf + 1, delim2, SSE_BATCH_SIZE - 1);
                        delims_in = _mm_loadu_si128((__m128i*)buf);
                    }

                    const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_MOST_SIGNIFICANT  | _SIDD_NEGATIVE_POLARITY;
                    do {
                        end -= SSE_BATCH_SIZE;
                        const int i = _mm_cmpistri(delims_in, _mm_load_si128((__m128i*)end), FLAGS);
                        if (i != (int)SSE_BATCH_SIZE)
                            return end + i + 1;
                    } while (end > align16);
                }
            }

            // Scan remaining chars
            while (str < end) {
                --end;
                if (!(*end == delim1 || *end == delim2))
                    return end + 1;
            }
        }
        return end;
    }

    ////

    // Scan to next delim
    inline const char* str_scan_delim_cpu(const char* str, const char* end, char delim1, char delim2) {
        if (str < end) {
            // Scan up to alignment boundary
            const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            while (str < align16)
                if (*str == delim1 || *str == delim2 || ++str == end)
                    return str;

            // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (str < align16) {
                __m128i delims_in;
                {
                    char buf[SSE_BATCH_SIZE];
                    buf[0] = delim1;
                    ::memset(buf + 1, delim2, SSE_BATCH_SIZE - 1);
                    delims_in = _mm_loadu_si128((__m128i*)buf);
                }

                const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT;
                do {
                    const int i = _mm_cmpistri(delims_in, _mm_load_si128((__m128i*)str), FLAGS);
                    if (i != (int)SSE_BATCH_SIZE)
                        return str + i;
                    str += SSE_BATCH_SIZE;
                } while (str < align16);
            }

            // Scan remaining chars
            for (; str < end; ++str)
                if (*str == delim1 || *str == delim2)
                    break;
        }
        return str;
    }

    // Scan to next delim in reverse, return new end
    inline const char* str_scan_delim_cpu_r(const char* str, const char* end, char delim1, char delim2) {
        if (str < end) {
            const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (align16 > str) {
                // Scan up to alignment boundary
                while (end > align16) {
                    --end;
                    if (*end == delim1 || *end == delim2)
                        return end + 1;
                }

                // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
                align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
                if (end > align16) {
                    __m128i delims_in;
                    {
                        char buf[SSE_BATCH_SIZE];
                        buf[0] = delim1;
                        ::memset(buf + 1, delim2, SSE_BATCH_SIZE - 1);
                        delims_in = _mm_loadu_si128((__m128i*)buf);
                    }

                    const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_MOST_SIGNIFICANT;
                    do {
                        end -= SSE_BATCH_SIZE;
                        const int i = _mm_cmpistri(delims_in, _mm_load_si128((__m128i*)end), FLAGS);
                        if (i != (int)SSE_BATCH_SIZE)
                            return end + i + 1;
                    } while (end > align16);
                }
            }

            // Scan remaining chars
            while (str < end) {
                --end;
                if (*end == delim1 || *end == delim2)
                    return end + 1;
            }
        }
        return end;
    }

    // Scan to next delim -- min 1 delim, max 16 delims
    inline const char* str_scan_delim_cpu(const char* str, const char* end, const char* delims, uint delim_count) {
        assert( delim_count > 1 );
        assert( delim_count <= SSE_BATCH_SIZE );
        if (str < end) {
            // Scan up to alignment boundary
            const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            while (str < align16)
                if (::memchr(delims, *str, delim_count) != NULL || ++str == end)
                    return str;

            // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (str < align16) {
                __m128i delims_in;
                {
                    char buf[SSE_BATCH_SIZE];
                    ::memcpy(buf, delims, delim_count);
                    if (delim_count < SSE_BATCH_SIZE)
                        ::memset(buf + delim_count, delims[0], SSE_BATCH_SIZE - delim_count);
                    delims_in = _mm_loadu_si128((__m128i*)buf);
                }

                const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT;
                do {
                    const int i = _mm_cmpistri(delims_in, _mm_load_si128((__m128i*)str), FLAGS);
                    if (i != (int)SSE_BATCH_SIZE)
                        return str + i;
                    str += SSE_BATCH_SIZE;
                } while (str < align16);
            }

            // Scan remaining chars
            for (; str < end; ++str)
                if (::memchr(delims, *str, delim_count) != NULL)
                    break;
        }
        return str;
    }

    // Scan to next delim in reverse, return new end -- min 1 delim, max 16 delims
    inline const char* str_scan_delim_cpu_r(const char* str, const char* end, const char* delims, uint delim_count) {
        if (str < end) {
            const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (align16 > str) {
                // Scan up to alignment boundary
                while (end > align16) {
                    --end;
                    if (::memchr(delims, *end, delim_count) != NULL)
                        return end + 1;
                }

                // Scan in chunks of batch size using SSE 4.2 instruction: pcmpistri
                align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
                if (end > align16) {
                    __m128i delims_in;
                    {
                        char buf[SSE_BATCH_SIZE];
                        ::memcpy(buf, delims, delim_count);
                        if (delim_count < SSE_BATCH_SIZE)
                            ::memset(buf + delim_count, delims[0], SSE_BATCH_SIZE - delim_count);
                        delims_in = _mm_loadu_si128((__m128i*)buf);
                    }

                    const int FLAGS = _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_MOST_SIGNIFICANT;
                    do {
                        end -= SSE_BATCH_SIZE;
                        const int i = _mm_cmpistri(delims_in, _mm_load_si128((__m128i*)end), FLAGS);
                        if (i != (int)SSE_BATCH_SIZE)
                            return end + i + 1;
                    } while (end > align16);
                }
            }

            // Scan remaining chars
            while (str < end) {
                --end;
                if (::memchr(delims, *end, delim_count) != NULL)
                    return end + 1;
            }
        }
        return end;
    }

#elif defined(EVO_IMPL_SSE2)
    // Scan to next non-whitespace char
    inline const char* str_scan_nws_cpu(const char* str, const char* end) {
        const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
        if (end > align16) {
            // Scan up to alignment boundary
            while (str < align16) {
                if (!(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r'))
                    return str;
                ++str;
            }

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (str < align16) {
                const char* WS = "                "
                    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
                    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                    "\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r";
                const __m128i ws1 = _mm_loadu_si128((__m128i*)WS);
                const __m128i ws2 = _mm_loadu_si128((__m128i*)(WS + SSE_BATCH_SIZE));
                const __m128i ws3 = _mm_loadu_si128((__m128i*)(WS + (SSE_BATCH_SIZE * 2)));
                const __m128i ws4 = _mm_loadu_si128((__m128i*)(WS + (SSE_BATCH_SIZE * 3)));

                __m128i v, n;
                uint16 r;
                do {
                    n = _mm_load_si128((__m128i*)str);

                    // pcmpeqb, por
                    v = _mm_or_si128(_mm_cmpeq_epi8(n, ws1), _mm_cmpeq_epi8(n, ws2));
                    v = _mm_or_si128(v, _mm_cmpeq_epi8(n, ws3));
                    v = _mm_or_si128(v, _mm_cmpeq_epi8(n, ws4));

                    // pmovmskb
                    r = (uint16)~_mm_movemask_epi8(v);
                    if (r != 0) {
                        #if defined(_MSC_VER)
                            ulong i;
                            _BitScanForward(&i, r);
                            return str + i;
                        #else
                            return str + __builtin_ffs(r) - 1;
                        #endif
                    }
                    str += SSE_BATCH_SIZE;
                } while (str < align16);
            }
        }

        // Scan remaining chars
        for (; str < end; ++str)
            if (!(*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r'))
                break;
        return str;
    }

    // Scan to next non-whitespace char in reverse, return new end
    inline const char* str_scan_nws_cpu_r(const char* str, const char* end) {
        const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
        if (align16 > str) {
            // Scan up to alignment boundary
            while (end > align16) {
                --end;
                if (!(*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
                    return end + 1;
            }

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            if (end > align16) {
                const char* WS = "                "
                    "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
                    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                    "\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r";
                const __m128i ws1 = _mm_loadu_si128((__m128i*)WS);
                const __m128i ws2 = _mm_loadu_si128((__m128i*)(WS + SSE_BATCH_SIZE));
                const __m128i ws3 = _mm_loadu_si128((__m128i*)(WS + (SSE_BATCH_SIZE * 2)));
                const __m128i ws4 = _mm_loadu_si128((__m128i*)(WS + (SSE_BATCH_SIZE * 3)));

                __m128i v, n;
                uint16 r;
                do {
                    end -= SSE_BATCH_SIZE;
                    n = _mm_load_si128((__m128i*)end);

                    // pcmpeqb, por
                    v = _mm_or_si128(_mm_cmpeq_epi8(n, ws1), _mm_cmpeq_epi8(n, ws2));
                    v = _mm_or_si128(v, _mm_cmpeq_epi8(n, ws3));
                    v = _mm_or_si128(v, _mm_cmpeq_epi8(n, ws4));

                    // pmovmskb
                    r = (uint16)~_mm_movemask_epi8(v);
                    if (r != 0) {
                        #if defined(_MSC_VER)
                            ulong i;
                            _BitScanReverse(&i, r);
                            return end + i + 1;
                        #else
                            return end + IntegerT<uint>::BITS - __builtin_clz(r);
                        #endif
                    }
                } while (end > align16);
            }
        }

        // Scan remaining chars
        while (str < end) {
            --end;
            if (!(*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
                return end + 1;
        }
        return end;
    }

    ////

    // Scan to next non-delim char
    inline const char* str_scan_ndelim_cpu(const char* str, const char* end, char delim1, char delim2) {
        const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
        if (end > align16) {
            // Scan up to alignment boundary
            while (str < align16)
                if (!(*str == delim1 || *str == delim2) || ++str == end)
                    return str;

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (str < align16) {
                __m128i delim_in1, delim_in2;
                {
                    char buf[SSE_BATCH_SIZE];
                    ::memset(buf, delim1, SSE_BATCH_SIZE);
                    delim_in1 = _mm_loadu_si128((__m128i*)buf);

                    ::memset(buf, delim2, SSE_BATCH_SIZE);
                    delim_in2 = _mm_loadu_si128((__m128i*)buf);
                }

                __m128i n;
                uint16 r;
                do {
                    // pcmpeqb, por, pmovmskb
                    n = _mm_load_si128((__m128i*)str);
                    r = (uint16)~_mm_movemask_epi8( _mm_or_si128(_mm_cmpeq_epi8(n, delim_in1), _mm_cmpeq_epi8(n, delim_in2)) );
                    if (r != 0) {
                        #if defined(_MSC_VER)
                            ulong i;
                            _BitScanForward(&i, r);
                            return str + i;
                        #else
                            return str + __builtin_ffs(r) - 1;
                        #endif
                    }
                    str += SSE_BATCH_SIZE;
                } while(str < align16);
            }
        }

        // Scan remaining chars
        for (; str < end; ++str)
            if (!(*str == delim1 || *str == delim2))
                break;
        return str;
    }

    // Scan to next non-delim char in reverse, return new end
    inline const char* str_scan_ndelim_cpu_r(const char* str, const char* end, char delim1, char delim2) {
        const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
        if (align16 > str) {
            // Scan up to alignment boundary
            while (end > align16) {
                --end;
                if (!(*end == delim1 || *end == delim2))
                    return end + 1;
            }

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            if (end > align16) {
                __m128i delim_in1, delim_in2;
                {
                    char buf[SSE_BATCH_SIZE];
                    ::memset(buf, delim1, SSE_BATCH_SIZE);
                    delim_in1 = _mm_loadu_si128((__m128i*)buf);

                    ::memset(buf, delim2, SSE_BATCH_SIZE);
                    delim_in2 = _mm_loadu_si128((__m128i*)buf);
                }

                __m128i n;
                uint16 r;
                do {
                    // pcmpeqb, por, pmovmskb
                    end -= SSE_BATCH_SIZE;
                    n = _mm_load_si128((__m128i*)end);
                    r = (uint16)~_mm_movemask_epi8( _mm_or_si128(_mm_cmpeq_epi8(n, delim_in1), _mm_cmpeq_epi8(n, delim_in2)) );
                    if (r != 0) {
                        #if defined(_MSC_VER)
                            ulong i;
                            _BitScanReverse(&i, r);
                            return end + i + 1;
                        #else
                            return end + IntegerT<uint>::BITS - __builtin_clz(r);
                        #endif
                    }
                } while (end > align16);
            }
        }

        // Scan remaining chars
        while (str < end) {
            --end;
            if (!(*end == delim1 || *end == delim2))
                return end + 1;
        }
        return end;
    }

    ////

    // Scan to next delim
    inline const char* str_scan_delim_cpu(const char* str, const char* end, char delim1, char delim2) {
        const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
        if (end > align16) {
            // Scan up to alignment boundary
            while (str < align16)
                if (*str == delim1 || *str == delim2 || ++str == end)
                    return str;

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (str < align16) {
                __m128i delim_in1, delim_in2;
                {
                    char buf[SSE_BATCH_SIZE];
                    ::memset(buf, delim1, SSE_BATCH_SIZE);
                    delim_in1 = _mm_loadu_si128((__m128i*)buf);

                    ::memset(buf, delim2, SSE_BATCH_SIZE);
                    delim_in2 = _mm_loadu_si128((__m128i*)buf);
                }

                __m128i n;
                uint16 r;
                do {
                    // pcmpeqb, por, pmovmskb
                    n = _mm_load_si128((__m128i*)str);
                    r = (uint16)_mm_movemask_epi8( _mm_or_si128(_mm_cmpeq_epi8(n, delim_in1), _mm_cmpeq_epi8(n, delim_in2)) );
                    if (r != 0) {
                        #if defined(_MSC_VER)
                            ulong i;
                            _BitScanForward(&i, r);
                            return str + i;
                        #else
                            return str + __builtin_ffs(r) - 1;
                        #endif
                    }
                    str += SSE_BATCH_SIZE;
                } while (str < align16);
            }
        }

        // Scan remaining chars
        for (; str < end; ++str)
            if (*str == delim1 || *str == delim2)
                break;
        return str;
    }

    // Scan to next delim -- min 1 delim, max 16 delims
    inline const char* str_scan_delim_cpu(const char* str, const char* end, const char* delims, uint delim_count) {
        assert( delim_count > 1 );
        assert( delim_count <= SSE_BATCH_SIZE );
        const char* align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
        if (end > align16) {
            // Scan up to alignment boundary
            while (str < align16)
                if (::memchr(delims, *str, delim_count) != NULL || ++str == end)
                    return str;

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
            if (str < align16) {
                uint i;
                __m128i delims_in[SSE_BATCH_SIZE];
                {
                    char buf[SSE_BATCH_SIZE];
                    for (i = 0; i < delim_count; ++i) {
                        ::memset(buf, delims[i], SSE_BATCH_SIZE);
                        delims_in[i] = _mm_loadu_si128((__m128i*)buf);
                    }
                }

                __m128i v, n;
                uint16 r;
                do {
                    n = _mm_load_si128((__m128i*)str);

                    // pcmpeqb, por
                    v = _mm_or_si128(_mm_cmpeq_epi8(n, delims_in[0]), _mm_cmpeq_epi8(n, delims_in[1]));
                    for (i = 2; i < delim_count; ++i)
                        v = _mm_or_si128(v, _mm_cmpeq_epi8(n, delims_in[i]));

                    // pmovmskb
                    r = (uint16)_mm_movemask_epi8(v);
                    if (r != 0) {
                    #if defined(_MSC_VER)
                        ulong j;
                        _BitScanForward(&j, r);
                        return str + j;
                    #else
                        return str + __builtin_ffs(r) - 1;
                    #endif
                    }
                    str += SSE_BATCH_SIZE;
                } while (str < align16);
            }
        }

        // Scan remaining chars
        for (; str < end; ++str)
            if (::memchr(delims, *str, delim_count) != NULL)
                break;
        return str;
    }

    // Scan to next delim in reverse, return new end
    inline const char* str_scan_delim_cpu_r(const char* str, const char* end, char delim1, char delim2) {
        const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
        if (align16 > str) {
            // Scan up to alignment boundary
            while (end > align16) {
                --end;
                if (*end == delim1 || *end == delim2)
                    return end + 1;
            }

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            if (end > align16) {
                __m128i delim_in1, delim_in2;
                {
                    char buf[SSE_BATCH_SIZE];
                    ::memset(buf, delim1, SSE_BATCH_SIZE);
                    delim_in1 = _mm_loadu_si128((__m128i*)buf);

                    ::memset(buf, delim2, SSE_BATCH_SIZE);
                    delim_in2 = _mm_loadu_si128((__m128i*)buf);
                }

                __m128i n;
                uint16 r;
                do {
                    // pcmpeqb, por, pmovmskb
                    end -= SSE_BATCH_SIZE;
                    n = _mm_load_si128((__m128i*)end);
                    r = (uint16)_mm_movemask_epi8( _mm_or_si128(_mm_cmpeq_epi8(n, delim_in1), _mm_cmpeq_epi8(n, delim_in2)) );
                    if (r != 0) {
                        #if defined(_MSC_VER)
                            ulong i;
                            _BitScanReverse(&i, r);
                            return end + i + 1;
                        #else
                            return end + IntegerT<uint>::BITS - __builtin_clz(r);
                        #endif
                    }
                } while (end > align16);
            }
        }

        // Scan remaining chars
        while (str < end) {
            --end;
            if (*end == delim1 || *end == delim2)
                return end + 1;
        }
        return end;
    }

    // Scan to next delim in reverse, return new end -- min 1 delim, max 16 delims
    inline const char* str_scan_delim_cpu_r(const char* str, const char* end, const char* delims, uint delim_count) {
        const char* align16 = (char*)((size_t)end & SSE_ALIGN16_MASK);
        if (align16 > str) {
            // Scan up to alignment boundary
            while (end > align16) {
                --end;
                if (::memchr(delims, *end, delim_count) != NULL)
                    return end + 1;
            }

            // Scan in chunks of batch size using SSE 2 instructions
            align16 = (char*)( ((size_t)str + SSE_ALIGN16) & SSE_ALIGN16_MASK );
            if (end > align16) {
                uint i;
                __m128i delims_in[SSE_BATCH_SIZE];
                {
                    char buf[SSE_BATCH_SIZE];
                    for (i = 0; i < delim_count; ++i) {
                        ::memset(buf, delims[i], SSE_BATCH_SIZE);
                        delims_in[i] = _mm_loadu_si128((__m128i*)buf);
                    }
                }

                __m128i v, n;
                uint16 r;
                do {
                    end -= SSE_BATCH_SIZE;
                    n = _mm_load_si128((__m128i*)end);

                    // pcmpeqb, por
                    v = _mm_or_si128(_mm_cmpeq_epi8(n, delims_in[0]), _mm_cmpeq_epi8(n, delims_in[1]));
                    for (i = 2; i < delim_count; ++i)
                        v = _mm_or_si128(v, _mm_cmpeq_epi8(n, delims_in[i]));

                    // pmovmskb
                    r = (uint16)_mm_movemask_epi8(v);
                    if (r != 0) {
                        #if defined(_MSC_VER)
                            ulong j;
                            _BitScanReverse(&j, r);
                            return end + j + 1;
                        #else
                            return end + IntegerT<uint>::BITS - __builtin_clz(r);
                        #endif
                    }
                } while (end > align16);
            }
        }

        // Scan remaining chars
        while (str < end) {
            --end;
            if (::memchr(delims, *end, delim_count) != NULL)
                return end + 1;
        }
        return end;
    }
#endif
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Scan string pointer for next non-whitespace character and return stop pointer.
 - \#include <evo/strscan.h>
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str  %String pointer to scan, must not be NULL
 \param  end  Pointer to stop scanning, must be >= `str`
 \return      Stop pointer, `end` if whole string was scanned, otherwise this points to the next non-whitespace character
*/
inline const char* str_scan_nws(const char* str, const char* end) {
    assert( str != NULL );
    assert( str <= end );
#if defined(EVO_CPU)
    return impl::str_scan_nws_cpu(str, end);
#else
    return impl::str_scan_nws_default(str, end);
#endif
}

/** Scan string pointer for next non-whitespace character in reverse and return new end after stop pointer.
 - \#include <evo/strscan.h>
 - This returns a new `end` pointer, which points to the character _after_ a non-whitespace
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str  %String pointer to scan, must not be NULL
 \param  end  End pointer to stop scanning, must be >= `str`
 \return      New end pointer, `str` if whole string was scanned, otherwise this points to the _next char after_ a non-whitespace character
*/
inline const char* str_scan_nws_r(const char* str, const char* end) {
    assert( str != NULL );
    assert( str <= end );
#if defined(EVO_CPU)
    return impl::str_scan_nws_cpu_r(str, end);
#else
    return impl::str_scan_nws_default_r(str, end);
#endif
}

/** Scan string pointer for next delimiter or non-whitespace character and return stop pointer.
 - \#include <evo/strscan.h>
 - This is similar to str_scan_nws(const char*,const char*) but also works when `delim` is itself a whitespace character
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str    %String pointer to scan, must not be NULL
 \param  end    Pointer to stop scanning, must be >= `str`
 \param  delim  Delimiter to stop at, which may be a whitespace character
 \return        Stop pointer, `end` if whole string was scanned, otherwise this points to the next non-whitespace character
*/
inline const char* str_scan_nws(const char* str, const char* end, char delim) {
    assert( str != NULL );
    assert( str <= end );
    switch (delim) {
        case ' ':
            while (str < end && (*str == '\t' || *str == '\r' || *str == '\n'))
                ++str;
            break;
        case '\t':
            while (str < end && (*str == ' ' || *str == '\r' || *str == '\n'))
                ++str;
            break;
        case '\r':
            while (str < end && (*str == ' ' || *str == '\t' || *str == '\n'))
                ++str;
            break;
        case '\n':
            while (str < end && (*str == ' ' || *str == '\t' || *str == '\r'))
                ++str;
            break;
        default:
        #if defined(EVO_CPU)
            return impl::str_scan_nws_cpu(str, end);
        #else
            return impl::str_scan_nws_default(str, end);
        #endif
    }
    return str;
}

/** Scan string pointer for next delimiter or non-whitespace character and return stop pointer.
 - \#include <evo/strscan.h>
 - This is similar to str_scan_nws(const char*,const char*) but also works when `delim` is itself a whitespace character
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str    %String pointer to scan, must not be NULL
 \param  end    Pointer to stop scanning, must be >= `str`
 \param  delim  Delimiter to stop at, which may be a whitespace character
 \return        Stop pointer, `end` if whole string was scanned, otherwise this points to the next non-whitespace character
*/
inline const char* str_scan_nws_r(const char* str, const char* end, char delim) {
    assert( str != NULL );
    assert( str <= end );
    switch (delim) {
        case ' ':
            while (--end > str && (*end == '\t' || *end == '\r' || *end == '\n'));
            ++end;
            break;
        case '\t':
            while (--end > str && (*end == ' ' || *end == '\r' || *end == '\n'));
            ++end;
            break;
        case '\r':
            while (--end > str && (*end == ' ' || *end == '\t' || *end == '\n'));
            ++end;
            break;
        case '\n':
            while (--end > str && (*end == ' ' || *end == '\t' || *end == '\r'));
            ++end;
            break;
        default:
        #if defined(EVO_CPU)
            return impl::str_scan_nws_cpu_r(str, end);
        #else
            return impl::str_scan_nws_default_r(str, end);
        #endif
    }
    return end;
}

/** Scan string pointer for next non-delimiter and return stop pointer.
 - \#include <evo/strscan.h>
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str     %String pointer to scan, must not be NULL
 \param  end     Pointer to stop scanning, must be >= `str`
 \param  delim1  First delimiter to recognize
 \param  delim2  Second delimiter to recognize
 \return         Stop pointer, `end` if whole string was scanned, otherwise this points to the next non-delimiter character
*/
inline const char* str_scan_ndelim(const char* str, const char* end, char delim1, char delim2) {
    assert( str != NULL );
    assert( str <= end );
#if defined(EVO_CPU)
    return impl::str_scan_ndelim_cpu(str, end, delim1, delim2);
#else
    return impl::str_scan_ndelim_default(str, end, delim1, delim2);
#endif
}

/** Scan string pointer for next non-delimiter in reverse and return new end after stop pointer.
 - \#include <evo/strscan.h>
 - This returns a new `end` pointer, which points to the character _after_ a non-delimiter
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str     %String pointer to scan, must not be NULL
 \param  end     End pointer to stop scanning, must be >= `str`
 \param  delim1  First delimiter to recognize
 \param  delim2  Second delimiter to recognize
 \return         New end pointer, `str` if whole string was scanned, otherwise this points to the _next char after_ a non-delimiter
*/
inline const char* str_scan_ndelim_r(const char* str, const char* end, char delim1, char delim2) {
    assert( str != NULL );
    assert( str <= end );
#if defined(EVO_CPU)
    return impl::str_scan_ndelim_cpu_r(str, end, delim1, delim2);
#else
    return impl::str_scan_ndelim_default_r(str, end, delim1, delim2);
#endif
}

/** Scan string pointer for next delimiter and return stop pointer.
 - \#include <evo/strscan.h>
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str     %String pointer to scan, must not be NULL
 \param  end     Pointer to stop scanning, must be >= `str`
 \param  delim1  First delimiter to recognize
 \param  delim2  Second delimiter to recognize
 \return         Stop pointer, `end` if whole string was scanned, otherwise this points to the next delimiter
*/
inline const char* str_scan_delim(const char* str, const char* end, char delim1, char delim2) {
    assert( str != NULL );
    assert( str <= end );
#if defined(EVO_CPU)
    return impl::str_scan_delim_cpu(str, end, delim1, delim2);
#else
    return impl::str_scan_delim_default(str, end, delim1, delim2);
#endif
}

/** Scan string pointer for next delimiter in reverse and return new end after stop pointer.
 - \#include <evo/strscan.h>
 - This returns a new `end` pointer, which points to the character _after_ a delimiter
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  str     %String pointer to scan, must not be NULL
 \param  end     End pointer to stop scanning, must be >= `str`
 \param  delim1  First delimiter to recognize
 \param  delim2  Second delimiter to recognize
 \return         New end pointer, `str` if whole string was scanned, otherwise this points to the _next char after_ a delimiter
*/
inline const char* str_scan_delim_r(const char* str, const char* end, char delim1, char delim2) {
    assert( str != NULL );
    assert( str <= end );
#if defined(EVO_CPU)
    return impl::str_scan_delim_cpu_r(str, end, delim1, delim2);
#else
    return impl::str_scan_delim_default_r(str, end, delim1, delim2);
#endif
}

/** Scan string pointer for next delimiter and return stop pointer.
 - \#include <evo/strscan.h>
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 - Note: SSE optimizations don't apply with `delim_count > 16`
 .
 \param  str          %String pointer to scan, must not be NULL
 \param  end          Pointer to stop scanning, must be >= `str`
 \param  delims       Delimiters to scan for
 \param  delim_count  Number of delimiter to scan for, must not be 0
 \return              Stop pointer, `end` if whole string was scanned, otherwise this points to the next delimiter
*/
inline const char* str_scan_delim(const char* str, const char* end, const char* delims, uint delim_count) {
    assert( str != NULL );
    assert( str <= end );
    assert( delim_count > 0 );
#if defined(EVO_CPU)
    switch (delim_count) {
        case 1: {
            str = (char*)::memchr(str, delims[0], end - str);
            return (str == NULL ? end : str);
        }
        case 2:
            return impl::str_scan_delim_cpu(str, end, delims[0], delims[1]);
        default:
            if (delim_count <= impl::SSE_BATCH_SIZE)
                return impl::str_scan_delim_cpu(str, end, delims, delim_count);
            break;
    }
#endif
    return impl::str_scan_delim_default(str, end, delims, delim_count);
}

/** Scan string pointer for next delimiter in reverse and return new end after stop pointer.
 - \#include <evo/strscan.h>
 - This returns a new `end` pointer, which points to the character _after_ a delimiter
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 - Note: SSE optimizations don't apply with `delim_count > 16`
 .
 \param  str          %String pointer to scan, must not be NULL
 \param  end          End pointer to stop scanning, must be >= `str`
 \param  delims       Delimiters to scan for
 \param  delim_count  Number of delimiter to scan for, must not be 0
 \return              New end pointer, `str` if whole string was scanned, otherwise this points to the _next char after_ a delimiter
*/
inline const char* str_scan_delim_r(const char* str, const char* end, const char* delims, uint delim_count) {
    assert( str != NULL );
    assert( str <= end );
    assert( delim_count > 0 );
#if defined(EVO_CPU)
    switch (delim_count) {
        case 1: {
            end = string_memrchr(str, delims[0], end - str);
            return (end == NULL ? str : end + 1);
        }
        case 2:
            return impl::str_scan_delim_cpu_r(str, end, delims[0], delims[1]);
        default:
            if (delim_count <= impl::SSE_BATCH_SIZE)
                return impl::str_scan_delim_cpu_r(str, end, delims, delim_count);
            break;
    }
#endif
    return impl::str_scan_delim_default_r(str, end, delims, delim_count);
}

/** Scan string for a Backtick + DEL pair.
 - \#include <evo/strscan.h>
 - A string is invalid text and is not quotable if it contains a backtick + DEL pair, and it contains all other quote types in a way that would confuse a parser
 - This is used by StrQuoting::get() methods when falling back to \link StrQuoting tBACKTICK_DEL::tBACKTICK_DEL\endlink
 - See \ref SmartQuoting
 .
 \param  str  %String pointer to scan, must not be NULL
 \param  end  Pointer to stop scanning, must be >= `str`
 \return      Whether a backtick + DEL pair was found
*/
inline bool str_scan_backtickdel(const char* str, const char* end) {
    assert( str != NULL );
    assert( str <= end );
    const char DEL_CHAR = 0x7F;
    --end; // exclude last char at first
    while (str < end) {
        str = (char*)::memchr(str, '`', end - str);
        if (str == NULL)
            break;
        if (*++str == DEL_CHAR)
            return true;
    }
    return false;
}

/** Scan string pointer and extract quoted or unquoted text.
 - \#include <evo/strscan.h>
 - This scans for the end of the quoted value and sets out params accordingly
 - \b Caution: This is dangerous if the input wasn't quoted _without_ a delimiter in mind -- see StrQuoting::get(const char*,ulong)
   - See also: str_scan_endq(const char*&,const char*&,const char*,const char*,char)
 - This supports all the Evo quoting types -- see \ref SmartQuoting
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  startq  %Set to start of quoted text (without quote chars)  [out]
 \param  endq    %Set to end of quoted text (without quote chars)  [out]
 \param  str     %String pointer to scan, must not be null
 \param  end     Pointer to stop scanning, must be >= `str`
 \return         Stop pointer, `end` if whole string was scanned
*/
inline const char* str_scan_endq(const char*& startq, const char*& endq, const char* str, const char* end) {
    if (str == end) {
        startq = str;
        endq   = end;
        return end;
    }
    assert( str < end );

    const char* p;
    switch (*str) {
        case '`': {
            const char DEL_CHAR = 0x7F;
            if (end >= str + 4 && str[1] == DEL_CHAR) {
                // Backtick + DEL quoted
                p = str + 2;
                for (;; ++p) {
                    p = (char*)::memchr(p, '`', end - p);
                    if (p == NULL || p + 1 >= end)
                        break;
                    if (p[1] == DEL_CHAR) {
                        startq = str + 2;
                        endq   = p;
                        return p + 2;
                    }
                }
            }
        } // fallthrough
        case '\'':
        case '"': {
            const char q = *str;
            if (end >= str + 6 && str[1] == q && str[2] == q) {
                // Triple char quoted
                p = str + 3;
                for (;;) {
                    p = (char*)::memchr(p, q, end - p);
                    if (p == NULL || p + 2 >= end)
                        break;
                    if (p[1] == q) {
                        if (p[2] == q) {
                            while (p + 3 < end && p[3] == q)
                                ++p; // include additional quote chars
                            startq = str + 3;
                            endq   = p;
                            return p + 3;
                        } else
                            p += 3;
                    } else
                        p += 2;
                }
            }

            // Single char quoted
            p = str + 1;
            p = (char*)::memchr(p, q, end - p);
            if (p == NULL)
                break;
            startq = str + 1;
            endq   = p;
            return p + 1;
        }
        default:
            break;
    }

    // Unquoted
    startq = str;
    endq   = end;
    return end;
}

/** Scan string pointer and extract quoted or unquoted text with a delimiter.
 - \#include <evo/strscan.h>
 - This scans for the end of the quoted value and sets out params accordingly
 - The quoted value ends with an end-quote, followed by optional whitespace (space, tab, newlines), followed by a delim or end of input
 - This supports all the Evo quoting types -- see \ref SmartQuoting
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 - See String::writequoted(), StrQuoting::get(bool&,const char*,ulong,char)
 .
 \param  startq  %Set to start of quoted text (without quote chars), same as `str` if unquoted  [out]
 \param  endq    %Set to end of quoted text (without quote chars)  [out]
 \param  str     %String pointer to scan, must not be null
 \param  end     Pointer to stop scanning, must be >= `str`
 \param  delim   Delimiter that ends the quoted text, may be a whitespace char (space, tab, newline)
 \return         Stop pointer, `end` if whole string was scanned, otherwise this will point to a delim
*/
inline const char* str_scan_endq(const char*& startq, const char*& endq, const char* str, const char* end, char delim) {
    if (str == end) {
        startq = str;
        endq   = end;
        return end;
    }
    assert( str < end );

    const char* p;
    switch (*str) {
        case '`': {
            const char DEL_CHAR = 0x7F;
            if (end >= str + 4 && str[1] == DEL_CHAR) {
                // Backtick + DEL quoted
                p = str + 2;
                for (;;) {
                    p = (char*)::memchr(p, '`', end - p);
                    if (p == NULL || p + 1 >= end)
                        break;
                    if (p[1] == DEL_CHAR) {
                        endq = p;
                        p = str_scan_nws(p + 2, end, delim);
                        if (p < end && *p != delim)
                            continue; // no delim/end here
                        startq = str + 2;
                        return p;
                    }
                    ++p;
                }
            }
        } // fallthrough
        case '\'':
        case '"': {
            const char q = *str;
            if (end >= str + 6 && str[1] == q && str[2] == q) {
                // Triple char quoted
                p = str + 3;
                for (;;) {
                    p = (char*)::memchr(p, q, end - p);
                    if (p == NULL || p + 2 >= end)
                        break;
                    if (p[1] == q) {
                        if (p[2] == q) {
                            while (p + 3 < end && p[3] == q)
                                ++p; // include additional quote chars
                            endq = p;
                            p = str_scan_nws(p + 3, end, delim);
                            if (p < end && *p != delim)
                                continue; // no delim/end here
                            startq = str + 3;
                            return p;
                        } else
                            p += 3;
                    } else
                        p += 2;
                }
            }

            // Single char quoted
            p = str + 1;
            for (;;) {
                p = (char*)::memchr(p, q, end - p);
                if (p == NULL)
                    break;
                endq = p;
                p = str_scan_nws(p + 1, end, delim);
                if (p < end && *p != delim)
                    continue; // no delim/end here
                startq = str + 1;
                return p;
            }
            break;
        }
        default:
            break;
    }

    // Unquoted
    startq = str;
    p = (char*)::memchr(str, delim, end - str);
    if (p == NULL) {
        endq = end;
        return end;
    }
    endq = p;
    return p;
}

/** Scan string pointer in reverse and extract quoted or unquoted text with a delimiter and return new end after stop pointer.
 - \#include <evo/strscan.h>
 - This scans in reverse (i.e. right to left):
   - This scans for the beginning of the quoted value and sets out params accordingly
   - The quoted value begins with an end-quote, preceded by optional whitespace (space, tab, newlines), preceded by a delim or end of input
   - This returns a new `end` pointer, which points to the character _after_ a delimiter
 - This supports all the Evo quoting types -- see \ref SmartQuoting
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 - See String::writequoted(), StrQuoting::get(bool&,const char*,ulong,char)
 .
 \param  startq  %Set to start of quoted text (without quote chars)  [out]
 \param  endq    %Set to end of quoted text (without quote chars), same as `end` if unquoted  [out]
 \param  str     %String beginning pointer to stop scan, must not be null
 \param  end     End pointer to start scanning in reverse, must be >= `str`
 \param  delim   Delimiter that ends the quoted text, may be a whitespace char (space, tab, newline)
 \return         New end pointer, `str` if whole string was scanned, otherwise this points to the _next char after_ a delimiter
*/
inline const char* str_scan_endq_r(const char*& startq, const char*& endq, const char* str, const char* end, char delim) {
    if (str == end) {
        startq = str;
        endq   = end;
        return str;
    }
    assert( str < end );

    const char DEL_CHAR = 0x7F;
    const char* p;
    switch (*(end - 1)) {
        case DEL_CHAR: {
            if (end >= str + 4 && *(end - 2) == '`') {
                p = end - 2;
                for (;;) {
                    p = string_memrchr(str, DEL_CHAR, p - str);
                    if (p == NULL || p == str)
                        break;
                    if (*--p == '`') {
                        startq = p + 2;
                        p = str_scan_nws_r(str, p, delim);
                        if (p > str && *(p - 1) != delim)
                            continue; // no delim/end here
                        endq = end - 2;
                        return p;
                    }
                }
            }
            break;
        }
        case '`':
        case '\'':
        case '"': {
            const char q = *(end - 1);
            if (end >= str + 6 && *(end - 2) == q && *(end - 3) == q) {
                // Triple char quoted
                p = end - 3;
                for (;;) {
                    p = string_memrchr(str, q, p - str);
                    if (p == NULL || p - 2 < str)
                        break;
                    if (*--p == q) {
                        if (*--p == q) {
                            while (p > str && *(p - 1) == q)
                                --p; // include additional quote chars
                            startq = p + 3;
                            p = str_scan_nws_r(str, p, delim);
                            if (p > str && *(p - 1) != delim)
                                continue; // no delim/end here
                            endq = end - 3;
                            return p;
                        }
                    }
                }
            }

            // Single char quoted
            p = end - 1;
            for (;;) {
                p = string_memrchr(str, q, p - str);
                if (p == NULL)
                    break;
                startq = p + 1;
                p = str_scan_nws_r(str, p, delim);
                if (p > str && *(p - 1) != delim)
                    continue; // no delim/end here
                endq = end - 1;
                return p;
            }
            break;
        }
        default:
            break;
    }

    // Unquoted
    endq = end;
    p = string_memrchr(str, delim, end - str);
    if (p == NULL) {
        startq = str;
        return str;
    }
    startq = p + 1;
    return startq;
}

/** Scan string pointer and extract quoted or unquoted text with 2 possible delimiters.
 - \#include <evo/strscan.h>
 - This scans for the end of the quoted value and sets out params accordingly
 - The quoted value ends with an end-quote, followed by optional whitespace (space, tab, newlines), followed by either delim or end of input
 - This supports all the Evo quoting types -- see \ref SmartQuoting
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 - See String::writequoted(), StrQuoting::get(bool&,const char*,ulong,char,char)
 .
 \param  startq  %Set to start of quoted text (without quote chars)  [out]
 \param  endq    %Set to end of quoted text (without quote chars)  [out]
 \param  str     %String pointer to scan, must not be null
 \param  end     Pointer to stop scanning, must be >= `str`
 \param  delim1  Delimiter that may end the quoted text, must not be a whitespace character (space, tab, newline)
 \param  delim2  Another delimiter that may end the quoted text, must not be a whitespace character (space, tab, newline)
 \return         Stop pointer, `end` if whole string was scanned, otherwise this will point to a delim
*/
inline const char* str_scan_endq(const char*& startq, const char*& endq, const char* str, const char* end, char delim1, char delim2) {
    if (str == end) {
        startq = str;
        endq   = end;
        return end;
    }
    assert( str < end );

    const char* p;
    switch (*str) {
        case '`': {
            const char DEL_CHAR = 0x7F;
            if (end >= str + 4 && str[1] == DEL_CHAR) {
                // Backtick + DEL quoted
                p = str + 2;
                for (;;) {
                    p = (char*)::memchr(p, '`', end - p);
                    if (p == NULL || p + 1 >= end)
                        break;
                    if (p[1] == DEL_CHAR) {
                        endq = p;
                        p = str_scan_nws(p + 2, end);
                        if (p < end && *p != delim1 && *p != delim2)
                            continue; // no delim/end here
                        startq = str + 2;
                        return p;
                    }
                    ++p;
                }
            }
        } // fallthrough
        case '\'':
        case '"': {
            const char q = *str;
            if (end >= str + 6 && str[1] == q && str[2] == q) {
                // Triple char quoted
                p = str + 3;
                for (;;) {
                    p = (char*)::memchr(p, q, end - p);
                    if (p == NULL || p + 2 >= end)
                        break;
                    if (p[1] == q) {
                        if (p[2] == q) {
                            while (p + 3 < end && p[3] == q)
                                ++p; // include additional quote chars
                            endq = p;
                            p = str_scan_nws(p + 3, end);
                            if (p < end && *p != delim1 && *p != delim2)
                                continue; // no delim/end here
                            startq = str + 3;
                            return p;
                        } else
                            p += 3;
                    } else
                        p += 2;
                }
            }

            // Single char quoted
            p = str + 1;
            for (;;) {
                p = (char*)::memchr(p, q, end - p);
                if (p == NULL)
                    break;
                endq = p;
                p = str_scan_nws(p + 1, end);
                if (p < end && *p != delim1 && *p != delim2)
                    continue; // no delim/end here
                startq = str + 1;
                return p;
            }
            break;
        }
        default:
            break;
    }

    // Unquoted
    startq = str;
    p = str_scan_delim(str, end, delim1, delim2);
    if (p == end) {
        endq = end;
        return end;
    }
    endq = p;
    return p;
}

/** Scan string pointer and extract quoted or unquoted text with multiple possible delimiters.
 - \#include <evo/strscan.h>
 - This scans for the end of the quoted value and sets out params accordingly
 - The quoted value ends with an end-quote, followed by optional whitespace (space, tab, newlines), followed by either a delim or end of input
 - This supports all the Evo quoting types -- see \ref SmartQuoting
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 - Note: SSE optimizations don't apply with `delim_count > 16`
 - See String::writequoted(), StrQuoting::get(bool&,const char*,ulong,char,char)
 .
 \param  startq       %Set to start of quoted text (without quote chars)  [out]
 \param  endq         %Set to end of quoted text (without quote chars)  [out]
 \param  str          %String pointer to scan, must not be null
 \param  end          Pointer to stop scanning, must be >= `str`
 \param  delims       Delimiters that may end the quoted text, if this contains a whitespace char (space, tab, newline) then `ws_delim` must be set too -- more than 1 whitespace delimiter is not supported
 \param  delim_count  Number of delimiters in `delims`, must be greater than 0
 \param  ws_delim     Use to specify a whitespace delimiter in `delims`, 0 if no whitespace delim -- this is used to handle whitespace correctly between an end-quote and a delimiter
 \return              Stop pointer, `end` if whole string was scanned, otherwise this will point to a delim
*/
inline const char* str_scan_endq(const char*& startq, const char*& endq, const char* str, const char* end, const char* delims, uint delim_count, char ws_delim=0) {
    if (str == end) {
        startq = str;
        endq   = end;
        return end;
    }
    assert( str < end );
    assert( delim_count > 0 );

    const char* p;
    switch (*str) {
        case '`': {
            const char DEL_CHAR = 0x7F;
            if (end >= str + 4 && str[1] == DEL_CHAR) {
                // Backtick + DEL quoted
                p = str + 2;
                for (;;) {
                    p = (char*)::memchr(p, '`', end - p);
                    if (p == NULL || p + 1 >= end)
                        break;
                    if (p[1] == DEL_CHAR) {
                        endq = p;
                        p = str_scan_nws(p + 2, end, ws_delim);
                        if (p < end && ::memchr(delims, *p, delim_count) == NULL)
                            continue; // no delim/end here
                        startq = str + 2;
                        return p;
                    }
                    ++p;
                }
            }
        } // fallthrough
        case '\'':
        case '"': {
            const char q = *str;
            if (end >= str + 6 && str[1] == q && str[2] == q) {
                // Triple char quoted
                p = str + 3;
                for (;;) {
                    p = (char*)::memchr(p, q, end - p);
                    if (p == NULL || p + 2 >= end)
                        break;
                    if (p[1] == q) {
                        if (p[2] == q) {
                            while (p + 3 < end && p[3] == q)
                                ++p; // include additional quote chars
                            endq = p;
                            p = str_scan_nws(p + 3, end, ws_delim);
                            if (p < end && ::memchr(delims, *p, delim_count) == NULL)
                                continue; // no delim/end here
                            startq = str + 3;
                            return p;
                        } else
                            p += 3;
                    } else
                        p += 2;
                }
            }

            // Single char quoted
            p = str + 1;
            for (;;) {
                p = (char*)::memchr(p, q, end - p);
                if (p == NULL)
                    break;
                endq = p;
                p = str_scan_nws(p + 1, end, ws_delim);
                if (p < end && ::memchr(delims, *p, delim_count) == NULL)
                    continue; // no delim/end here
                startq = str + 1;
                return p;
            }
            break;
        }
        default:
            break;
    }

    // Unquoted
    startq = str;
    p = str_scan_delim(str, end, delims, delim_count);
    if (p == end) {
        endq = end;
        return end;
    }
    endq = p;
    return p;
}

/** Scan string pointer in reverse and extract quoted or unquoted text with multiple possible delimiters and return new end after stop pointer.
 - \#include <evo/strscan.h>
 - This scans in reverse (i.e. right to left):
   - This scans for the beginning of the quoted value and sets out params accordingly
   - The quoted value begins with an end-quote, preceded by optional whitespace (space, tab, newlines), preceded by a delim or end of input
   - This returns a new `end` pointer, which points to the character _after_ a delimiter
 - This supports all the Evo quoting types -- see \ref SmartQuoting
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 - Note: SSE optimizations don't apply with `delim_count > 16`
 - See String::writequoted(), StrQuoting::get(bool&,const char*,ulong,char)
 .
 \param  startq       %Set to start of quoted text (without quote chars)  [out]
 \param  endq         %Set to end of quoted text (without quote chars), same as `end` if unquoted  [out]
 \param  str          %String beginning pointer to stop scan, must not be null
 \param  end          End pointer to start scanning in reverse, must be >= `str`
 \param  delims       Delimiters that may end the quoted text, if this contains a whitespace char (space, tab, newline) then `ws_delim` must be set too -- more than 1 whitespace delimiter is not supported
 \param  delim_count  Number of delimiters in `delims`, must be greater than 0
 \param  ws_delim     Use to specify a whitespace delimiter in `delims`, 0 if no whitespace delim -- this is used to handle whitespace correctly between an end-quote and a delimiter
 \return              New end pointer, `str` if whole string was scanned, otherwise this points to the _next char after_ a delimiter
*/
inline const char* str_scan_endq_r(const char*& startq, const char*& endq, const char* str, const char* end, const char* delims, uint delim_count, char ws_delim=0) {
    if (str == end) {
        startq = str;
        endq   = end;
        return str;
    }
    assert( str < end );
    assert( delim_count > 0 );

    const char DEL_CHAR = 0x7F;
    const char* p;
    switch (*(end - 1)) {
        case DEL_CHAR: {
            if (end >= str + 4 && *(end - 2) == '`') {
                p = end - 2;
                for (;;) {
                    p = string_memrchr(str, DEL_CHAR, p - str);
                    if (p == NULL || p == str)
                        break;
                    if (*--p == '`') {
                        startq = p + 2;
                        p = str_scan_nws_r(str, p, ws_delim);
                        if (p > str && ::memchr(delims, *(p - 1), delim_count) == NULL)
                            continue; // no delim/end here
                        endq = end - 2;
                        return p;
                    }
                }
            }
            break;
        }
        case '`':
        case '\'':
        case '"': {
            const char q = *(end - 1);
            if (end >= str + 6 && *(end - 2) == q && *(end - 3) == q) {
                // Triple char quoted
                p = end - 3;
                for (;;) {
                    p = string_memrchr(str, q, p - str);
                    if (p == NULL || p - 2 < str)
                        break;
                    if (*--p == q) {
                        if (*--p == q) {
                            while (p > str && *(p - 1) == q)
                                --p; // include additional quote chars
                            startq = p + 3;
                            p = str_scan_nws_r(str, p, ws_delim);
                            if (p > str && ::memchr(delims, *(p - 1), delim_count) == NULL)
                                continue; // no delim/end here
                            endq = end - 3;
                            return p;
                        }
                    }
                }
            }

            // Single char quoted
            p = end - 1;
            for (;;) {
                p = string_memrchr(str, q, p - str);
                if (p == NULL)
                    break;
                startq = p + 1;
                p = str_scan_nws_r(str, p, ws_delim);
                if (p > str && ::memchr(delims, *(p - 1), delim_count) == NULL)
                    continue; // no delim/end here
                endq = end - 1;
                return p;
            }
            break;
        }
        default:
            break;
    }

    // Unquoted
    endq = end;
    p = str_scan_delim_r(str, end, delims, delim_count);
    if (p == str) {
        startq = str;
        return str;
    }
    startq = p;
    return startq;
}

/** Scan string pointer for char and return stop pointer.
 - \#include <evo/strscan.h>
 .
 \param  maxlen  Max allowed length to scan before returning an error, ignored if 0
 \param  str     %String pointer to scan, must not be NULL
 \param  end     Pointer to stop scanning, must be >= str
 \param  ch      %Char to scan for
 \return         Stop pointer, `end` if whole string was scanned, NULL if `maxlen` exceeded and `ch` not found within `maxlen`
*/
inline const char* str_scan_to(uint maxlen, const char* str, const char* end, char ch) {
    if (str == end)
        return end;
    assert( str < end );
    if (maxlen > 0) {
        const char* maxlen_end = str + maxlen;
        if (maxlen_end < end)
            return (char*)::memchr(str, ch, maxlen_end - str);
    }
    str = (char*)::memchr(str, ch, end - str);
    if (str == NULL)
        return end;
    return str;
}

/** Scan string pointer for either of 2 chars and return stop pointer.
 - \#include <evo/strscan.h>
 - This uses SSE optimization when enabled -- see \ref CppCompilers
 .
 \param  maxlen  Max allowed length to scan before returning an error, ignored if 0
 \param  str     %String pointer to scan, must not be NULL
 \param  end     Pointer to stop scanning, must be >= str
 \param  ch1     First char to scan for
 \param  ch2     Second char to scan for
 \return         Stop pointer, `end` if whole string was scanned, NULL if maxlen exceeded
*/
inline const char* str_scan_to(uint maxlen, const char* str, const char* end, char ch1, char ch2) {
    if (str == end)
        return end;
    assert( str < end );
    if (maxlen > 0) {
        const char* maxlen_end = str + maxlen;
        if (maxlen_end < end) {
            str = str_scan_delim(str, maxlen_end, ch1, ch2);
            if (str < maxlen_end)
                return str;
            return NULL;
        }
    }
    return str_scan_delim(str, end, ch1, ch2);
}

/** Scan string pointer for decimal number and return stop pointer.
 - \#include <evo/strscan.h>
 .
 \param  num  Stores parsed number (not reset to 0)  [in/out]
 \param  str  %String pointer to scan, must not be NULL
 \param  end  Pointer to stop scanning, must be >= str
 \return      Stop pointer, end if whole string was scanned, NULL if invalid number or too many digits
*/
template<class T>
inline const char* str_scan_decimal(T& num, const char* str, const char* end) {
    const T BASE = 10;
    const T limitbase = IntegerT<T>::MAX / BASE;
    const T limitdig  = IntegerT<T>::MAX % BASE;
    char ch;
    const char* p = str;
    for (; p < end; ++p) {
        ch = *p;
        if (ch < '0' || ch > '9')
            return p;
        ch -= '0';
        if (num > limitbase || (num == limitbase && (T)ch > limitdig))
            return NULL;
        num *= BASE;
        num += (T)ch;
    }
    if (p == str)
        return NULL;
    return p;
}

/** Scan string pointer for hex number and return stop pointer.
 - \#include <evo/strscan.h>
 .
 \param  num  Stores parsed number (not reset to 0)  [in/out]
 \param  str  %String pointer to scan, must not be NULL
 \param  end  Pointer to stop scanning, must be >= str
 \return      Stop pointer, end if whole string was scanned, NULL if invalid number or too many digits
*/
template<class T>
inline const char* str_scan_hex(T& num, const char* str, const char* end) {
    const T BASE = 16;
    const T limitbase = IntegerT<T>::MAX / BASE;
    char ch;
    const char* p = str;
    for (; p < end; ++p) {
        ch = *p;
        if (ch <= '9') {
            if (ch < '0')
                return p;
            ch -= '0';
        } else if (ch >= 'a') {
            if (ch > 'f')
                return p;
            ch -= ('a' - 10);
        } else if (ch <= 'F') {
            if (ch < 'A')
                return p;
            ch -= ('A' - 10);
        } else
            return p;
        if (num > limitbase)
            return NULL;
        num *= BASE;
        num += (T)ch;
    }
    if (p == str)
        return NULL;
    return p;
}

///////////////////////////////////////////////////////////////////////////////

/** Helpers for determining quoting type to use with string data.
 - Used by methods like String::writequoted()
 - See \ref SmartQuoting
*/
struct StrQuoting {
    static const char DEL_CHAR = 0x7F;  ///< DEL character

    /** Quoting type. */
    enum Type {
        tSINGLE,            ///< Single-quotes: <code> ' </code>
        tDOUBLE,            ///< Double-quotes: <code> " </code>
        tBACKTICK,          ///< Backtick: <code> \` </code>
        tSINGLE3,           ///< Triple single-quotes: <code> ''' </code>
        tDOUBLE3,           ///< Triple double-quotes: <code> """ </code>
        tBACKTICK3,         ///< Triple backtick: <code> ``` </code>
        tBACKTICK_DEL,      ///< Backtick followed by DEL char (7F) -- last resort (rare)
        tERROR              ///< Data not quotable (invalid text)
    };

    /* Scan string data and determine required quoting type to make it parsable.
     - This scans the string for quote characters -- for best performance specify a delimiter with: get(bool&,const char*,ulong,char)
     - This uses Smart Quoting -- see \ref SmartQuoting
     .
     \param  data       %String data pointer
     \param  data_size  %String size
     \param  delim      Delimiter that may follow the quoted string
     \return            Quoting type for string data
    */
    static Type get(const char* data, ulong data_size) {
        if (data_size == 0)
            return tSINGLE;
        assert( data != NULL );

        const uint DELIMS_COUNT = 3;
        const char* DELIMS = "'\"`";
        const char* save_ptr[DELIMS_COUNT];

        // Check for quote chars, save quote pointer for triple-quote checks below
        const char* p;
        for (ulong i = 0; i < DELIMS_COUNT; ++i) {
            p = (char*)::memchr(data, DELIMS[i], data_size);
            if (p == NULL)
                return (Type)i;
            save_ptr[i] = p;
        }

        // All quote chars found, look for unused triple-quotes
        const char* end = data + data_size - 2;
        for (ulong i = 0; i < DELIMS_COUNT; ++i) {
            const char ch = DELIMS[i];
            p = save_ptr[i];
            for (;;) {
                if (p < end) {
                    if (p[1] == ch && p[2] == ch)
                        break;
                    if ((p = (char*)::memchr(p + 1, ch, end - p - 1)) != NULL)
                        continue;
                }
                return (Type)(i + DELIMS_COUNT);
            }
        }

        // Fallback to backtick + del (rare)
        if (str_scan_backtickdel(data, data + data_size))
            return tERROR;
        return tBACKTICK_DEL;
    }

    /* Scan string data and determine required quoting type to make it parsable.
     - This scans the string for the delimiter and checks quote chars before all delimiters to determine the best quoting type to use
     - This uses Smart Quoting -- see \ref SmartQuoting
     .
     \param  optional   %Set to whether string data may be left unquoted  [out]
     \param  data       %String data pointer
     \param  data_size  %String size
     \param  delim      Delimiter a parser expects after the quoted string
     \return            Quoting type for string data, tERROR if data is unquotable (invalid text)
    */
    static Type get(bool& optional, const char* data, ulong data_size, char delim) {
        if (data_size == 0) {
            optional = true;
            return tSINGLE;
        } else if (data_size == 1) {
            optional = (*data != delim && *data > ' ');
            return (*data == '\'' ? tDOUBLE : tSINGLE);
        }
        assert( data != NULL );

        bool block_backtick_del = false;
        const char* end = data + data_size;
        const char* p = (char*)::memchr(data, delim, data_size);
        if (p == NULL) {
            // No delim, check first and last chars to determine quoting
            switch (*data) {
                case '\'':
                    optional = false;
                    if (*(end - 1) == '"')
                        return tBACKTICK;
                    return tDOUBLE;
                case '"':
                    optional = false;
                    if (*(end - 1) == '\'')
                        return tBACKTICK;
                    return tSINGLE;
                case '`':
                    optional = false;
                    if (*(end - 1) == '\'')
                        return tDOUBLE;
                    return tSINGLE;
                default:
                    switch (*(end - 1)) {
                        case '\'':
                            optional = false;
                            return tDOUBLE;
                        case '"':
                        case '`':
                            optional = false;
                            return tSINGLE;
                        default:
                            optional = (*data > ' ' && *(end - 1) > ' ');
                            return tSINGLE;
                    }
            }
        } else {
            enum {
                ALLOW = 0, // ok to use
                BLOCK,     // ruled out, do not use
                AVOID      // avoid, use if all others are blocked
            };
            const uint QUOTING_SIZE = 6;
            char quoting[QUOTING_SIZE] = { 0, 0, 0, 0, 0, 0 }; // value per quote type, '"` then same as triples
            char ch;
            optional = false;

            // Quotes used at end should be avoided
            ch = *(end - 1);
            switch (ch) {
                case '\'':
                    quoting[(uint)tSINGLE] = (char)AVOID;
                    quoting[(uint)tSINGLE3] = (char)AVOID;
                    break;
                case '"':
                    quoting[(uint)tDOUBLE] = (char)AVOID;
                    quoting[(uint)tDOUBLE3] = (char)AVOID;
                    break;
                case '`':
                    quoting[(uint)tBACKTICK] = (char)AVOID;
                    quoting[(uint)tBACKTICK3] = (char)AVOID;
                    break;
                default:
                    break;
            }

            // Quotes used at beginning should be avoided, block single quote char if ambiguous
            ch = *data;
            switch (ch) {
                case '\'':
                    if (data_size > 1 && data[1] == ch)
                        quoting[(uint)tSINGLE]  = (char)BLOCK;
                    else
                        quoting[(uint)tSINGLE]  = (char)AVOID;
                    quoting[(uint)tSINGLE3] = (char)AVOID;
                    break;
                case '"':
                    if (data_size > 1 && data[1] == ch)
                        quoting[(uint)tDOUBLE]  = (char)BLOCK;
                    else
                        quoting[(uint)tDOUBLE]  = (char)AVOID;
                    quoting[(uint)tDOUBLE3] = (char)AVOID;
                    break;
                case '`':
                    if (data_size > 1 && data[1] == ch)
                        quoting[(uint)tBACKTICK]  = (char)BLOCK;
                    else
                        quoting[(uint)tBACKTICK]  = (char)AVOID;
                    quoting[(uint)tBACKTICK3] = (char)AVOID;
                    break;
                default:
                    break;
            }

            // Block quote types found right before a delim
            const char* data_plus_3 = data + 3;
            do {
                if (p > data) {
                    ch = *(p - 1);
                    switch (ch) {
                        case '\'':
                            quoting[(uint)tSINGLE] = (char)BLOCK;
                            if (p >= data_plus_3 && *(p - 2) == ch && *(p - 3) == ch)
                                quoting[(uint)tSINGLE3] = (char)BLOCK;
                            break;
                        case '"':
                            quoting[(uint)tDOUBLE] = 1;
                            if (p >= data_plus_3 && *(p - 2) == ch && *(p - 3) == ch)
                                quoting[(uint)tDOUBLE3] = (char)BLOCK;
                            break;
                        case '`':
                            quoting[(uint)tBACKTICK] = 1;
                            if (p >= data_plus_3 && *(p - 2) == ch && *(p - 3) == ch)
                                quoting[(uint)tBACKTICK3] = (char)BLOCK;
                            break;
                        case DEL_CHAR:
                            if (p >= data + 2 && *(p - 2) == '`')
                                block_backtick_del = true;
                            break;
                        default:
                            break;
                    }
                }
                ++p;
                p = (char*)::memchr(p, delim, end - p);
            } while (p != NULL);

            // Use first allowed quoting type
            p = (char*)::memchr(quoting, ALLOW, QUOTING_SIZE);
            if (p != NULL)
                return (Type)(p - quoting);

            // Try first non-preferred quoting type (marked AVOID but not BLOCKED)
            p = (char*)::memchr(quoting, AVOID, QUOTING_SIZE);
            if (p != NULL)
                return (Type)(p - quoting);
        }

        // Fallback to backtick + del (rare)
        if (block_backtick_del)
            return tERROR;
        return tBACKTICK_DEL;
    }

    /* Scan string data and determine required quoting type to make it parsable.
     - This scans the string for the delimiter and checks quote chars before all delimiters to determine the best quoting type to use
     - This uses Smart Quoting -- see \ref SmartQuoting
     .
     \param  data       %String data pointer
     \param  data_size  %String size
     \param  delim      Delimiter a parser expects after the quoted string
     \return            Quoting type for string data, tERROR if data is unquotable (invalid text)
    */
    static Type get(const char* data, ulong data_size, char delim) {
        bool optional;
        return get(optional, data, data_size, delim);
    }

    /* Scan string data and determine required quoting type to make it parsable.
     - This scans the string for a delimiter and checks quote chars before all delimiters to determine the best quoting type to use
     - This uses Smart Quoting -- see \ref SmartQuoting
     .
     \param  data       %String data pointer
     \param  data_size  %String size
     \param  delim1     Delimiter a parser may expect after the quoted string
     \param  delim2     Another delimiter a parser may expect after the quoted string
     \return            Quoting type for string data, tERROR if data is unquotable (invalid text)
    */
    static Type get(const char* data, ulong data_size, char delim1, char delim2) {
        if (data_size == 0)
            return tSINGLE;
        else if (data_size == 1)
            return (*data == '\'' ? tDOUBLE : tSINGLE);
        assert( data != NULL );

        bool block_backtick_del = false;
        const char* end = data + data_size;
        const char* p = str_scan_delim(data, end, delim1, delim2);
        if (p >= end) {
            // No delim, check first and last chars to determine quoting
            switch (*data) {
                case '\'':
                    if (*(end - 1) == '"')
                        return tBACKTICK;
                    return tDOUBLE;
                case '"':
                    if (*(end - 1) == '\'')
                        return tBACKTICK;
                    return tSINGLE;
                case '`':
                    if (*(end - 1) == '\'')
                        return tDOUBLE;
                    return tSINGLE;
                default:
                    switch (*(end - 1)) {
                        case '\'':
                            return tDOUBLE;
                        case '"':
                        case '`':
                            return tSINGLE;
                        default:
                            return tSINGLE;
                    }
            }
        } else {
            enum {
                ALLOW = 0, // ok to use
                BLOCK,     // ruled out, do not use
                AVOID      // avoid, use if all others are blocked
            };
            const uint QUOTING_SIZE = 6;
            char quoting[QUOTING_SIZE] = { 0, 0, 0, 0, 0, 0 }; // value per quote type, '"` then same as triples
            char ch;

            // Quotes used at end should be avoided
            ch = *(end - 1);
            switch (ch) {
                case '\'':
                    quoting[(uint)tSINGLE] = (char)AVOID;
                    quoting[(uint)tSINGLE3] = (char)AVOID;
                    break;
                case '"':
                    quoting[(uint)tDOUBLE] = (char)AVOID;
                    quoting[(uint)tDOUBLE3] = (char)AVOID;
                    break;
                case '`':
                    quoting[(uint)tBACKTICK] = (char)AVOID;
                    quoting[(uint)tBACKTICK3] = (char)AVOID;
                    break;
                default:
                    break;
            }

            // Quotes used at beginning should be avoided, block single quote char if ambiguous
            ch = *data;
            switch (ch) {
                case '\'':
                    if (data_size > 1 && data[1] == ch)
                        quoting[(uint)tSINGLE]  = (char)BLOCK;
                    else
                        quoting[(uint)tSINGLE]  = (char)AVOID;
                    quoting[(uint)tSINGLE3] = (char)AVOID;
                    break;
                case '"':
                    if (data_size > 1 && data[1] == ch)
                        quoting[(uint)tDOUBLE]  = (char)BLOCK;
                    else
                        quoting[(uint)tDOUBLE]  = (char)AVOID;
                    quoting[(uint)tDOUBLE3] = (char)AVOID;
                    break;
                case '`':
                    if (data_size > 1 && data[1] == ch)
                        quoting[(uint)tBACKTICK]  = (char)BLOCK;
                    else
                        quoting[(uint)tBACKTICK]  = (char)AVOID;
                    quoting[(uint)tBACKTICK3] = (char)AVOID;
                    break;
                default:
                    break;
            }

            // Block quote types found right before a delim
            const char* data_plus_3 = data + 3;
            do {
                if (p > data) {
                    ch = *(p - 1);
                    switch (ch) {
                        case '\'':
                            quoting[(uint)tSINGLE] = (char)BLOCK;
                            if (p >= data_plus_3 && *(p - 2) == ch && *(p - 3) == ch)
                                quoting[(uint)tSINGLE3] = (char)BLOCK;
                            break;
                        case '"':
                            quoting[(uint)tDOUBLE] = 1;
                            if (p >= data_plus_3 && *(p - 2) == ch && *(p - 3) == ch)
                                quoting[(uint)tDOUBLE3] = (char)BLOCK;
                            break;
                        case '`':
                            quoting[(uint)tBACKTICK] = 1;
                            if (p >= data_plus_3 && *(p - 2) == ch && *(p - 3) == ch)
                                quoting[(uint)tBACKTICK3] = (char)BLOCK;
                            break;
                        case DEL_CHAR:
                            if (p >= data + 2 && *(p - 2) == '`')
                                block_backtick_del = true;
                            break;
                        default:
                            break;
                    }
                }
                p = str_scan_delim(++p, end, delim1, delim2);
            } while (p < end);

            // Use first allowed quoting type
            p = (char*)::memchr(quoting, ALLOW, QUOTING_SIZE);
            if (p != NULL)
                return (Type)(p - quoting);

            // Try first non-preferred quoting type (marked AVOID but not BLOCKED)
            p = (char*)::memchr(quoting, AVOID, QUOTING_SIZE);
            if (p != NULL)
                return (Type)(p - quoting);
        }

        // Fallback to backtick + del (rare)
        if (block_backtick_del)
            return tERROR;
        return tBACKTICK_DEL;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
