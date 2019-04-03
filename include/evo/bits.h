// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file bits.h Evo bit manipulation. */
#pragma once
#ifndef INCL_evo_bits_h
#define INCL_evo_bits_h

#include "type.h"

// Disable certain MSVC warnings for this file
#if defined(_MSC_VER)
    #include <intrin.h>
    #pragma warning(push)
    #pragma warning(disable:4146)
#endif

namespace evo {
/** \addtogroup EvoBitManipulation
Evo bit manipulation helpers
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl {
#if defined(__GNUC__) && !defined(EVO_NOBUILTIN_BITS) && !defined(EVO_GCC_NOBUILTIN_POPCOUNT)
    template<class T,int S=sizeof(T)> struct BitsPopCount {
        static int popcount(uint32 mask) {
            return (uint)__builtin_popcount(mask);
        }
    };
    template<class T> struct BitsPopCount<T,4> {
        static int popcount(uint32 mask) {
        #if defined(EVO_64)
            return (uint)__builtin_popcount(mask);
        #else
            return (uint)__builtin_popcountl(mask);
        #endif
        }
    };
    template<class T> struct BitsPopCount<T,8> {
        static int popcount(uint64 mask) {
            return (uint)__builtin_popcountll(mask);
        }
    };
#else
    template<class T,int S=sizeof(T)> struct BitsPopCount {
        static int popcount(uint32 mask) {
            mask -= ((mask >> 1) & 0x55555555);
            mask = (mask & 0x33333333) + ((mask >> 2) & 0x33333333);
            return (int)((((mask + (mask >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24);
        }
    };
    template<class T> struct BitsPopCount<T,8> {
        static int popcount(uint64 mask) {
            return BitsPopCount<uint32>::popcount((uint32)(mask & 0xFFFFFFFFUL)) +
                   BitsPopCount<uint32>::popcount((uint32)((mask >> 32) & 0xFFFFFFFFUL));
        }
    };
#endif
}
/** \endcond */

#if defined(_MSC_VER) && !defined(EVO_NOBUILTIN_BITS)
    namespace impl {
        template<class T,int S=sizeof(T)> struct BitsClz {
            // Not used: INVALID_INTEGER_TYPE
        };
        template<class T> struct BitsClz<T,1> {
            static uint clz(uint8 mask) {
                ulong i;
                if (_BitScanReverse(&i, (uint32)mask) == 0)
                    return NONE;
                return (uint)(7 - i);
            }
        };
        template<class T> struct BitsClz<T,2> {
            static uint clz(uint16 mask) {
                ulong i;
                if (_BitScanReverse(&i, (uint32)mask) == 0)
                    return NONE;
                return (uint)(15 - i);
            }
        };
        template<class T> struct BitsClz<T,4> {
            static uint clz(uint32 mask) {
                ulong i;
                if (_BitScanReverse(&i, mask) == 0)
                    return NONE;
                return (uint)(31 - i);
            }
        };
        template<class T> struct BitsClz<T,8> {
            static uint clz(uint64 mask) {
                ulong i;
            #if defined(EVO_64)
                if (_BitScanReverse64(&i, mask) == 0)
                    return NONE;
                return (uint)(63 - i);
            #else
                if (_BitScanReverse(&i, ((mask >> 32) & 0x00000000FFFFFFFFUL)) != 0)
                    return (uint)(31 - i);
                else if (_BitScanReverse(&i, (mask & 0x00000000FFFFFFFFUL)) != 0)
                    return (uint)(63 - i);
                return NONE;
            #endif
            }
        };
    }

    inline bool bits_cpu_popcnt() {
    #if defined(_M_ARM)
        return false; // Not supported by ARM
    #else
        int cpuinfo[4];
        __cpuid(cpuinfo, 0x00000001);
        return (cpuinfo[2] & 0x800000) != 0;
    #endif
    }
#elif defined(__GNUC__) && !defined(EVO_NOBUILTIN_BITS)
    namespace impl {
        template<class T,int S=sizeof(T)> struct BitsClz {
            // Not used: INVALID_INTEGER_TYPE
        };
        template<class T> struct BitsClz<T,1> {
            static uint clz(uint8 mask) {
                const uint OFFSET = (sizeof(uint) * 8) - 8;
                if (mask == 0)
                    return NONE;
                return (uint)__builtin_clz((uint)mask) - OFFSET;
            }
        };
        template<class T> struct BitsClz<T,2> {
            static uint clz(uint16 mask) {
                const uint OFFSET = (sizeof(uint) * 8) - 16;
                if (mask == 0)
                    return NONE;
                return (uint)__builtin_clz((uint)mask) - OFFSET;
            }
        };
        template<class T> struct BitsClz<T,4> {
            static uint clz(uint32 mask) {
                if (mask == 0)
                    return NONE;
            #if defined(EVO_64)
                return (uint)__builtin_clz(mask);
            #else
                return (uint)__builtin_clzl(mask);
            #endif
            }
        };
        template<class T> struct BitsClz<T,8> {
            static uint clz(uint64 mask) {
                if (mask == 0)
                    return NONE;
                return (uint)__builtin_clzll(mask);
            }
        };
    }

    inline bool bits_cpu_popcnt() {
        return false;
    }
#else
    /** \cond impl */
    namespace impl {
        template<class T,int S=sizeof(T)> struct BitsClz {
            // Not used: INVALID_INTEGER_TYPE
        };
        template<class T> struct BitsClz<T,1> {
            static uint clz(uint8 mask) {
                if (mask == 0)
                    return NONE;
                mask |= (mask >> 1);
                mask |= (mask >> 2);
                mask |= (mask >> 4);
                return 8 - BitsPopCount<T>::popcount(mask);
            }
        };
        template<class T> struct BitsClz<T,2> {
            static uint clz(uint16 mask) {
                if (mask == 0)
                    return NONE;
                mask |= (mask >> 1);
                mask |= (mask >> 2);
                mask |= (mask >> 4);
                mask |= (mask >> 8);
                return 16 - BitsPopCount<T>::popcount(mask);
            }
        };
        template<class T> struct BitsClz<T,4> {
            static uint clz(uint32 mask) {
                if (mask == 0)
                    return NONE;
                mask |= (mask >> 1);
                mask |= (mask >> 2);
                mask |= (mask >> 4);
                mask |= (mask >> 8);
                mask |= (mask >> 16);
                return 32 - BitsPopCount<T>::popcount(mask);
            }
        };
        template<class T> struct BitsClz<T,8> {
            static uint clz(uint64 mask) {
                if (mask == 0)
                    return NONE;
                mask |= (mask >> 1);
                mask |= (mask >> 2);
                mask |= (mask >> 4);
                mask |= (mask >> 8);
                mask |= (mask >> 16);
                mask |= (mask >> 32);
                return 64 - BitsPopCount<T>::popcount(mask);
            }
        };
    }
    /** \endcond */

    /** Runtime check whether current CPU supports the POPCNT instruction.
     - MSVC: Checks whether CPU for this instruction
       - See: https://docs.microsoft.com/en-us/cpp/intrinsics/popcnt16-popcnt-popcnt64
     - Other compilers: Always returns false
     .
     \return  Whether POPCNT instruction is supported
     \see bits_popcount()
    */
    inline bool bits_cpu_popcnt() {
        return false;
    }
#endif

/** Get population count (number of bits set) for value.
 \tparam  T  Mask type (inferred from argument)
 \param  mask  Bit mask to use
 \return       Number of bits set in mask
*/
template<class T>
inline int bits_popcount(T mask) {
    return impl::BitsPopCount<T>::popcount(mask);
}

/** Get leading zero count on bitmask.
 - \b Caution: Different input sizes can give different results on same value so fixed-size functions like bits_clz32() are often preferred
 .
 \tparam  T  Mask type, inferred by param `mask`
 \param  mask  Input bitmask
 \return       Leading zero count, NONE if mask is 0
 \see bits_clz8(), bits_clz16(), bits_clz32(), bits_clz64()
*/
template<class T>
inline uint bits_clz(T mask) {
    return impl::BitsClz<T>::clz(mask);
}

/** Get leading zero count on bitmask (uint8).
 \tparam  T  Mask type, inferred by param `mask`
 \param  mask  Input bitmask
 \return       Leading zero count, NONE if mask is 0
 \see bits_clz(), bits_clz16(), bits_clz32(), bits_clz64()
*/
inline uint bits_clz8(uint8 mask) {
    return impl::BitsClz<uint8>::clz(mask);
}

/** Get leading zero count on bitmask (uint16).
 \tparam  T  Mask type, inferred by param `mask`
 \param  mask  Input bitmask
 \return       Leading zero count, NONE if mask is 0
 \see bits_clz(), bits_clz8(), bits_clz32(), bits_clz64()
*/
inline uint bits_clz16(uint16 mask) {
    return impl::BitsClz<uint16>::clz(mask);
}

/** Get leading zero count on bitmask (uint32).
 \tparam  T  Mask type, inferred by param `mask`
 \param  mask  Input bitmask
 \return       Leading zero count, NONE if mask is 0
 \see bits_clz(), bits_clz8(), bits_clz16(), bits_clz64()
*/
inline uint bits_clz32(uint32 mask) {
    return impl::BitsClz<uint32>::clz(mask);
}

/** Get leading zero count on bitmask (uint64).
 \tparam  T  Mask type, inferred by param `mask`
 \param  mask  Input bitmask
 \return       Leading zero count, NONE if mask is 0
 \see bits_clz(), bits_clz8(), bits_clz16(), bits_clz32()
*/
inline uint bits_clz64(uint64 mask) {
    return impl::BitsClz<uint64>::clz(mask);
}

///////////////////////////////////////////////////////////////////////////////

/** Traits and helpers for bit manipulation.

See BitArray for a bit array container.

\tparam  T      Chunk value type for storing bits, must be an unsigned integer type or results are undefined
\tparam  TSize  Size integer type, must be unsigned
*/
template<class T=ulong,class TSize=SizeT>
struct Bits {
    typedef T     Value;                        ///< Chunk value type
    typedef TSize Size;                         ///< Size integer type

    static const uint BYTES        = sizeof(T);             ///< Number of bytes for type `T`
    static const uint BITS         = BYTES * 8;             ///< Number of bits for type `T`
    static const uint BITS_MINUS_1 = BITS - 1;              ///< BITS minus 1, used by bit manipulation code
    static const T    RBIT         = 0x01;                  ///< Mask with right-most bit set
    static const T    LBIT         = RBIT << BITS_MINUS_1;  ///< Mask with left-most bit set
    static const T    ZERO         = 0;                     ///< Zero value
    static const T    ALLBITS      = T(~ZERO);              ///< Value with all bits set

    /** Align bits on right to the left.
     - This shifts so `count` bits on the right are aligned on the left
     - Vacated bits on the right are zero-filled
     .
     \param  value  Value to adjust
     \param  count  Number of bits on the right side to align to the left side
     \return        Aligned value, or same value if `count` is too high
    */
    static T align_left(T value, uint count) {
        if (count >= BITS)
            return value;
        return value << (BITS - count);
    }

    /** Align bits on left to the right.
     - This shifts so `count` bits on the left are aligned on the right
     - Vacated bits on the left are zero-filled
     .
     \param  value  Value to adjust
     \param  count  Number of bits on the left side to align to the right side
     \return        Aligned value, or same value if `count` is too high
     */
    static T align_right(T value, uint count) {
        if (count >= BITS)
            return value;
        return value >> (BITS - count);
    }

    /** Get bitmask with `count` bits set from `start` position.
     - Bits are counted left-to-right, as if from an array of bools
     - \b Caution: Results are undefined if start or count are out of bounds, see safemask() for safer version
     .
     \param  start  Mask start position, 0 for first bit on left, etc
     \param  count  Number of bits to have set from start
     \return        Mask value with given bits set
     \see safemask()
    */
    static T mask(uint start, uint count) {
        assert( start + count <= BITS );
        if (start > 0)
            return (((RBIT << count) - 1) << (BITS - (start + count)));
        return -(LBIT >> (count - 1));
    }

    /** Get bitmask with `count` bits set from `start` position, with bounds checking for safety.
     - Bits are counted left-to-right, as if from an array of bools
     - This is slightly less efficient than mask() due to added bounds checking
     .
     \param  start  Mask start position, 0 for first bit on left, etc
     \param  count  Number of bits to have set from start, adjusted if out of bounds
     \return        Mask value with given bits set, 0 if `start` out of bounds or `count` is 0
     \see mask()
    */
    static T safemask(uint start, uint count) {
        if (start < BITS && count > 0) {
            const uint end = start + count;
            if (end <= BITS) {
                // Copied from mask()
                if (start > 0)
                    return (((RBIT << count) - 1) << (BITS - end));
                return -(LBIT >> (count - 1));
            } else if (start > 0)
                return (LBIT >> (start - 1)) - 1;
            return ALLBITS;
        }
        return ZERO;
    }

    /** Calculate array size in chunks for number of bits.
     - This figures the number of chunks needed to store given bitcount
     .
     \param  bitsize  Number of bits in bit array
     \return          Number of bit array chunks needed for bitsize
     \see array_bitsize()
    */
    static Size array_size(Size bitsize) {
        return (bitsize + BITS_MINUS_1) / BITS;
    }

    /** Calculate array size in bits for number of chunks.
     - This is the reverse of array_size()
     .
     \param  chunks  Number of chunks in bit array
     \return         Number of bits for number of chunks
     \see array_size()
    */
    static Size array_bitsize(Size chunks) {
        return (chunks * BITS);
    }

    /** Count number of set bits in bit array.
     - This scans and counts the number of bits set
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \return          Number of bits set, 0 if none
    */
    static Size array_countbits(const T* data, Size bitsize) {
        assert( !IntegerT<T>::SIGN );
        Size count = 0;
        for (Size pos = 0, i = 0; pos < bitsize; pos += BITS, ++i)
            count += bits_popcount(data[i]);
        assert( count <= bitsize );
        return count;
    }

    /** Count number of set bits in subset of bit array.
     - This scans and counts the number of bits set in the subset
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to start at, 0 for first bit on left, etc
     \param  count    Number of bits to count from pos, ALL for all from pos
     \return          Number of bits set in subset, 0 if none
    */
    static Size array_countbits(const T* data, Size bitsize, Size pos, Size count) {
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize && count > 0) {
            // bitsize becomes maxcount
            bitsize -= pos;
            if (count > bitsize)
                count = bitsize;

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            const T* p = data + index;
            if (offset_end <= BITS) {
                // All in 1 chunk
                const T maskval = (count == BITS ? ALLBITS : (((RBIT << count) - 1) << (BITS - offset_end)));
                return bits_popcount(*p & maskval);
            } else {
                Size result = 0;

                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    result += bits_popcount(*p & ((LBIT >> (offset - 1)) - 1));
                    ++p;
                }

                // Remaining chunks
                while (count >= BITS) {
                    count -= BITS;
                    result += bits_popcount(*p & ALLBITS);
                    ++p;
                }

                // Last partial chunk
                if (count > 0)
                    result += bits_popcount(*p & -(LBIT >> (count - 1)));

                return result;
            }
        }
        return 0;
    }

    /** Check if all bits are set in bit array.
     - This scans and checks that all bits are set, and stops early if a bit isn't set
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \return          Whether all bits are set, false if a bit isn't set, true if empty
    */
    static bool array_checkall(const T* data, Size bitsize) {
        assert( !IntegerT<T>::SIGN );
        Size i = 0, sz = bitsize / BITS;
        for (; i < sz; ++i)
            if (data[i] != ALLBITS)
                return false;
        Size pos = bitsize - (i * BITS);
        if (pos > 0)
            if ((data[i] | ((LBIT >> (pos - 1)) - 1)) != ALLBITS)
                return false;
        return true;
    }

    /** Check if all bits are set in subset of bit array.
     - This scans and checks that all bits are set in the subset, and stops early if a bit isn't set
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to start at, 0 for first bit on left, etc
     \param  count    Number of bits to check from pos, ALL for all from pos
     \return          Whether all bits in subset are set, false if a bit isn't set, true if subset is empty
    */
    static bool array_checkall(const T* data, Size bitsize, Size pos, Size count) {
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize && count > 0) {
            // bitsize becomes maxcount
            bitsize -= pos;
            if (count > bitsize)
                count = bitsize;

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            const T* p = data + index;
            if (offset_end <= BITS) {
                // All in 1 chunk
                const T maskval = (count == BITS ? ALLBITS :  (((RBIT << count) - 1) << (BITS - offset_end)));
                return (*p | (T)~maskval) == ALLBITS;
            } else {
                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    if ((*p | (T)~((LBIT >> (offset - 1)) - 1)) != ALLBITS)
                        return false;
                    ++p;
                }

                // Remaining chunks
                while (count >= BITS) {
                    count -= BITS;
                    if (*p != ALLBITS)
                        return false;
                    ++p;
                }

                // Last partial chunk
                return (count == 0 || (*p | (T)~-(LBIT >> (count - 1))) == ALLBITS);
            }
        }
        return true;
    }

    /** Check if any bits are set in bit array.
     - This scans and checks that at least 1 bit is set, and stops early if a bit is set
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \return          Whether at least 1 bit is set, false if all bits are cleared
    */
    static bool array_checkany(const T* data, Size bitsize) {
        for (Size pos = 0, i = 0; pos < bitsize; pos += BITS, ++i)
            if (data[i] != 0)
                return true;
        return false;
    }

    /** Check if any bits are set in subset of bit array.
     - This scans and checks that at least 1 bit is set in the subset, and stops early if a bit is set
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to start at, 0 for first bit on left, etc
     \param  count    Number of bits to check from pos, ALL for all from pos
     \return          Whether at least 1 bit in subset is set, false if all bits in subset are cleared
    */
    static bool array_checkany(const T* data, Size bitsize, Size pos, Size count) {
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize && count > 0) {
            // bitsize becomes maxcount
            bitsize -= pos;
            if (count > bitsize)
                count = bitsize;

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            const T* p = data + index;
            if (offset_end <= BITS) {
                // All in 1 chunk
                const T maskval = (count == BITS ? ALLBITS :  (((RBIT << count) - 1) << (BITS - offset_end)));
                return (*p & maskval) != ZERO;
            } else {
                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    if ((*p & (T)((LBIT >> (offset - 1)) - 1)) != ZERO)
                        return true;
                    ++p;
                }

                // Remaining chunks
                while (count >= BITS) {
                    count -= BITS;
                    if (*p != ZERO)
                        return true;
                    ++p;
                }

                // Last partial chunk
                return (count != 0 && (*p & (T)-(LBIT >> (count - 1))) != ZERO);
            }
        }
        return false;
    }

    /** Bit array iteration state.
     - Used with array_iter() and array_iternext()
     - This is a low-level struct with public members, members should not be accessed directly
    */
    struct IterState {
        const T* data;
        Size size;
        Size index;
        Size pos;
        T chunk;
    
        IterState() {
            data = NULL;
            size = 0;
            index = NONE;
            pos   = 0;
            chunk = 0;
        }
    };

    /** Iterate to first set bit in array.
     - Bits are counted left-to-right, as if from an array of bools
     - Call this to find the first set bit, then call array_iternext() with same state repeatedly to find the remaining set bits
     .
     \param  state    Stores iteration state for calling array_iternext() for next set bit  [out]
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \return          Bit position for first set bit (from left, 0 for first, 1 for second, etc), NONE if no bits set
     \see array_iternext()
    */
    static Size array_iter(IterState& state, const T* data, Size bitsize) {
        state.data = data;
        state.size = (bitsize + BITS - 1) / BITS;
        for (Size i = 0, sz = state.size; i < sz; ++i) {
            if (data[i] != 0) {
                state.chunk = data[i];
                state.index = i;
                state.pos   = bits_clz(state.chunk);
                return state.pos + (i * BITS);
            }
        }
        state.index = NONE;
        return NONE;
    }

    /** Iterate to next set bit in array.
     - Bits are counted left-to-right, as if from an array of bools
     - Call array_iter() first to find first set bit, then call this with same state repeatedly to find the remaining set bits
     .
     \param  state  Stores iteration state while iterating  [in/out]
     \return        Bit position for next set bit (from left, 0 for first, 1 for second, etc), NONE if no more bits set
     \see array_iter()
    */
    static Size array_iternext(IterState& state) {
        Size i  = state.index;
        Size sz = state.size;
        if (i < sz) {
            // Resume from previous bit
            if (state.pos < BITS_MINUS_1) {
                state.chunk &= ((LBIT >> state.pos) - 1);
                if (state.chunk != 0) {
                    // Next bit is in same chunk
                    state.pos = bits_clz(state.chunk);
                    return state.pos + (i * BITS);
                }
            }
            ++i;

            // Check remaining chunks
            for (const T* data = state.data; i < sz; ++i) {
                if (data[i] != 0) {
                    state.chunk = data[i];
                    state.index = i;
                    state.pos   = bits_clz(state.chunk);
                    return state.pos + (i * BITS);
                }
            }
            state.index = NONE;
        }
        return NONE;
    }

    /** Get bit at position from chunked bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to get, 0 for first bit on left, etc
     \return          Bit value, false if `pos` is out of bounds
     \see array_set()
    */
    static bool array_get(const T* data, Size bitsize, Size pos) {
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize) {
            const Size i = (pos / BITS);
            return ( data[i] & (LBIT >> (pos - (i * BITS))) ) != 0;
        }
        return false;
    }

    /** %Set or clear bit at position in chunked bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to set, 0 for first bit on left, etc
     \param  value    Bit value to set, true to set bit, false to clear bit
     \return          Whether successfull, false if `pos` out of bounds
     \see array_set_multi(), array_toggle(), array_get()
    */
    static bool array_set(T* data, Size bitsize, Size pos, bool value=true) {
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize) {
            const Size i = (pos / BITS);
            const T mask = (LBIT >> (pos - (i * BITS)));
            if (value)
                data[i] |= mask;
            else
                data[i] &= ~mask;
            return true;
        }
        return false;
    }

    /** %Set or clear count bits at position in chunked bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to set, 0 for first bit on left, etc
     \param  count    Number of bits to set from pos, ALL for all from pos
     \param  value    Bit value to set, true to set bits, false to clear bits
     \return          Number of bits modified, 0 if out of bounds or count=0
     \see array_set()
    */
    static Size array_set_multi(T* data, Size bitsize, Size pos=0, Size count=ALL, bool value=true) {
    #if defined(EVO_OLDCC) // Fixes undefined reference on older compilers
        const T ALLBITS = Bits<T,SizeT>::ALLBITS;
        const T ZERO    = Bits<T,SizeT>::ZERO;
    #endif
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize && count > 0) {
            // bitsize becomes return value
            bitsize -= pos;
            if (count > bitsize)
                count = bitsize;
            else
                bitsize = count;

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            T* p = data + index;
            T maskval;
            if (offset_end <= BITS) {
                // All in 1 chunk
                maskval = (count == BITS ? ALLBITS :  (((RBIT << count) - 1) << (BITS - offset_end)));
                if (value)
                    *p |= maskval;
                else
                    *p &= ~maskval;
            } else {
                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    maskval = (LBIT >> (offset - 1)) - 1;
                    if (value)
                        *p |= maskval;
                    else
                        *p &= ~maskval;
                    ++p;
                }

                // Remaining chunks
                maskval = (value ? ALLBITS : ZERO);
                while (count >= BITS) {
                    count -= BITS;
                    *p = maskval;
                    ++p;
                }

                // Last partial chunk
                if (count > 0) {
                    maskval = -(LBIT >> (count - 1));
                    if (value)
                        *p |= maskval;
                    else
                        *p &= ~maskval;
                }
            }
            return bitsize;
        }
        return 0;
    }

    /** Toggle bit at position in chunked bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to toggle, 0 for first bit on left, etc
     \return          Whether successfull, false if `pos` out of bounds
     \see array_toggle_multi(), array_set(), array_get()
    */
    static bool array_toggle(T* data, Size bitsize, Size pos) {
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize) {
            const Size i = (pos / BITS);
            data[i] ^= (LBIT >> (pos - (i * BITS)));
            return true;
        }
        return false;
    }

    /** Toggle count bits at position in chunked bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to toggle, 0 for first bit on left, etc
     \param  count    Number of bits to toggle from pos, ALL for all from pos
     \return          Number of bits modified, 0 if out of bounds or count=0
     \see array_set()
    */
    static Size array_toggle_multi(T* data, Size bitsize, Size pos=0, Size count=ALL) {
        assert( !IntegerT<T>::SIGN );
        if (pos < bitsize && count > 0) {
            // bitsize becomes return value
            bitsize -= pos;
            if (count > bitsize)
                count = bitsize;
            else
                bitsize = count;

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            T* p = data + index;
            if (offset_end <= BITS) {
                // All in 1 chunk
                const T maskval = (count == BITS ? ALLBITS :  (((RBIT << count) - 1) << (BITS - offset_end)));
                *p ^= maskval;
            } else {
                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    *p ^= (LBIT >> (offset - 1)) - 1;
                    ++p;
                }

                // Remaining chunks
                while (count >= BITS) {
                    count -= BITS;
                    *p ^= ALLBITS;
                    ++p;
                }

                // Last partial chunk
                if (count > 0)
                    *p ^= -(LBIT >> (count - 1));
            }
            return bitsize;
        }
        return 0;
    }

    /** Stores bits in chunked bit array.
     - %Bits are stored in chunks but are positioned left-to-right, as if in an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Value bits are taken from the right so integers can be stored
       - Example value `00000111` with `count=3` would store `111` (3 right-most bits) or `7` in decimal
     - %Bits stored after end of bit array are truncated and lost
     - \b Caution: If count exceeds `value` size, count is capped at that size -- only bits from `value` are stored
     .
     \tparam  U  Value type to store, signed value is treated as unsigned -- inferred from `value` param
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to store at, 0 for first bit on left, etc
     \param  count    Number of bits to store from value
     \param  value    Value to store (`count` right-most bits are stored)
     \return          Whether successful, false if `pos` is out of bounds
     \see array_set(), array_extract()
    */
    template<class U>
    static bool array_store(T* data, Size bitsize, Size pos, Size count, U value) {
        assert( !IntegerT<T>::SIGN );
        if (count > 0) {
            if (count > (Size)IntegerT<U>::BITS)
                count = (Size)IntegerT<U>::BITS;

            if (pos >= bitsize)
                return false;
            bitsize -= pos;

            typename ToUnsigned<U>::Type uvalue = (typename ToUnsigned<U>::Type)value;

            if (count < (Size)IntegerT<U>::BITS)
                // Mask out bits not being stored
                uvalue &= ((IntegerT<U>::RBIT << count) - 1);

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            T* p = data + index;
            T maskval;
            if (offset_end <= BITS) {
                // All in 1 chunk
                const uint lshift = BITS - offset_end;
                maskval = (count == BITS ? 0 : ~(((RBIT << count) - 1) << lshift));
                *p = (*p & maskval) | (T(uvalue) << lshift);
            } else {
                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    maskval = (LBIT >> (offset - 1)) - 1;
                    *p = (*p & ~maskval) | (T(uvalue >> count) & maskval);
                    ++p;
                }

                // Remaining chunks
                while (count >= BITS) {
                    count -= BITS;
                    *p = T(uvalue >> count);
                    ++p;
                }

                // Last partial chunk
                if (count > 0) {
                    maskval = -(LBIT >> (count - 1));
                    *p = (*p & ~maskval) | ((T(uvalue) << (BITS - count)) & maskval);
                }
            }
            return true;
        }
        return false;
    }

    /** Extract bits from chunked bit array.
     - %Bits are stored in chunks but are extracted left-to-right, as if from an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Extracted bits are left-aligned in the return value, and the left-most bit is the "first" bit (as if from an array of bools)
       - Example with 3 extracted bits `111` returned as `uint8`: `11100000`
     .
     \tparam  U  Return type for extracted bits, must be unsigned (uint, ulong, uint32, etc) -- defaults to `ulong` in C++11 or newer
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to extract from, 0 for first bit on left, etc
     \param  count    Number of bits to extract, truncated if larger than bitsize or return value
     \return          Extracted bits, aligned to the left
     \see array_extractr(), array_get(), array_store()
    */
    template<class U EVO_ONCPP11(=uint32)>
    static U array_extractl(const T* data, Size bitsize, Size pos, Size count) {
        assert( !IntegerT<T>::SIGN );
        assert( !IntegerT<U>::SIGN );
        if (pos < bitsize && count > 0) {
            bitsize -= pos;
            if (count > bitsize)
                count = bitsize;
            if (count > (Size)IntegerT<U>::BITS)
                count = (Size)IntegerT<U>::BITS;

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            const T* ptr = data + index;

            if (offset_end <= BITS) {
                // All in 1 chunk
                if (count == BITS)
                    return U(*ptr); // whole chunk
                return U( (*ptr & (T(~(ALLBITS >> count)) >> offset)) >> (BITS - offset_end) )
                    << ((Size)IntegerT<U>::BITS - count); // left align
            } else {
                // Use bitsize to store extracted bitsize
                bitsize = count;
                U result;

                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    result = U(*ptr & (ALLBITS >> offset)) << count;
                    ++ptr;
                } else
                    result = 0;

                // Whole chunks
                while (count >= BITS) {
                    count -= BITS;
                    result |= U(*ptr) << count;
                    ++ptr;
                }

                // Last partial chunk
                if (count > 0)
                    result |= (*ptr & ~(ALLBITS >> count)) >> (BITS - count);

                // Left align
                return result << ((Size)IntegerT<U>::BITS - bitsize);
            }
        }
        return 0;
    }

    /** Extract bits from chunked bit array.
     - %Bits are stored in chunks but are extracted left-to-right, as if from an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Extracted bits returned are right-aligned so the result is an integer
       - Example with 3 extracted bits `111` returned as `uint8`: `00000111` or `7` in decimal
     - A `count` larger than bits in return type will be truncated -- _this should be avoided_ as it may not be easy to tell how many bits were truncated
       - For example, if you pass `count=9` with return type `uint8` then this uses `count=8`
     - Otherwise `count` may go out of bit array bounds without truncation -- bits from out of bounds are set to 0 and still are right-aligned according to `count`
     .
     \tparam  U  Return type for extracted bits, must be unsigned (uint, ulong, uint32, etc) -- defaults to `ulong` in C++11 or newer
     \param  data     Bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  bitsize  Number of bits in bit array
     \param  pos      Bit position to extract from, 0 for first bit on left, etc
     \param  count    Number of bits to extract, ALL for num bits in return type, truncated if larger than bits in return type
     \return          Extracted bits, aligned to the right
     \see array_extractl(), array_get(), array_store()
    */
    template<class U EVO_ONCPP11(=uint32)>
    static U array_extractr(const T* data, Size bitsize, Size pos, Size count) {
        assert( !IntegerT<T>::SIGN );
        assert( !IntegerT<U>::SIGN );
        if (pos < bitsize && count > 0) {
            if (count > (Size)IntegerT<U>::BITS)
                count = (Size)IntegerT<U>::BITS;
            bitsize -= pos;

            Size truncbits = 0;
            if (count > bitsize) {
                // Pretend to extract bits after end of array
                truncbits = count - bitsize;
                count = bitsize;
            }

            Size       index  = (pos / BITS);
            const Size offset = pos - (index * BITS);
            const Size offset_end = offset + count;
            const T* ptr = data + index;
            U result;

            if (offset_end <= BITS) {
                // All in 1 chunk
                if (count == BITS)
                    result = U(*ptr);
                else
                    result = U( (*ptr & (T(~(ALLBITS >> count)) >> offset)) >> (BITS - offset_end) );
            } else {
                // First partial chunk
                if (offset > 0) {
                    count -= (BITS - offset);
                    result = U(*ptr & (ALLBITS >> offset)) << count;
                    ++ptr;
                } else
                    result = 0;

                // Whole chunks
                while (count >= BITS) {
                    count -= BITS;
                    result |= U(*ptr) << count;
                    ++ptr;
                }

                // Last partial chunk
                if (count > 0)
                    result |= (*ptr & ~(ALLBITS >> count)) >> (BITS - count);
            }

            assert( truncbits < (Size)IntegerT<U>::BITS );
            return result << truncbits;
        }
        return 0;
    }

    /** Copy bits from another array.
     - Bits are counted left-to-right, as if from an array of bools
     - Bits that don't fit will be truncated
     .
     \param  data         Bit array data pointer to copy to, treated as a bool array even though bits come in chunks of T
     \param  bitsize      Number of bits in bit array to copy to
     \param  src_data     Source bit array data pointer, treated as a bool array even though bits come in chunks of T
     \param  src_bitsize  Number of bits in source bit array
     \param  src_pos      Source bit position to copy from, 0 for first bit on left, etc
     \param  src_count    Number of bits to copy
     \return              Number of bits copied, may be truncated if out of bounds
     \see array_extract(), array_store()
    */
    static Size array_copy(T* data, Size bitsize, T* src_data, Size src_bitsize, Size src_pos=0, Size src_count=ALL) {
        assert( !IntegerT<T>::SIGN );
        if (src_pos < src_bitsize && src_count > 0) {
            if (src_count > bitsize)
                src_count = bitsize;

            // src_bitsize becomes return value
            src_bitsize -= src_pos;
            if (src_count > src_bitsize)
                src_count = src_bitsize;
            else
                src_bitsize = src_count;

            Size       src_index  = (src_pos / BITS);
            const Size src_offset = src_pos - (src_index * BITS);
            const Size src_offset_end = src_offset + src_count;
            const T* src_p = src_data + src_index;
            if (src_offset_end <= BITS) {
                // All in 1 chunk
                *data = (*data & ~(ALLBITS << src_offset)) | ((*src_p & (ALLBITS >> src_offset)) << src_offset);
            } else if (src_offset > 0) {
                // Copy each chunk while reading with offset
                const Size leadbits = (BITS - src_offset);
                T maskval1 = ALLBITS >> src_offset;
                T maskval2 = ~maskval1;
                while (src_count >= BITS) {
                    src_count -= BITS;
                    *data = ((*src_p & maskval1) << src_offset) | ((src_p[1] & maskval2) >> leadbits);
                    ++src_p;
                    ++data;
                }
                // Last partial chunk
                if (src_count > 0) {
                    maskval1 = ALLBITS >> src_count;
                    maskval2 = ~maskval1;
                    *data = (*data & maskval1) | ((*src_p << src_offset) & maskval2);
                }
            } else {
                // Copy full chunks
                while (src_count >= BITS) {
                    src_count -= BITS;
                    *data = *src_p;
                    ++data;
                    ++src_p;
                }
                // Last partial chunk
                if (src_count > 0) {
                    const T maskval = ALLBITS >> src_count;
                    *data = (*data & maskval) | (*src_p & ~maskval);
                }
            }
            return src_bitsize;
        }
        return 0;
    }

    /** Shift to the left in chunked bit array.
     - New vacated bits on the right are zero-filled
     .
     \param  data     Bit array data pointer
     \param  bitsize  Number of bits in bit array
     \param  count    Count to shift, must not be negative
    */
    static void array_shiftl(T* data, Size bitsize, uint count) {
        assert( !IntegerT<T>::SIGN );
        assert( count >= 0 );
        if (bitsize > 0) {
            if (count >= bitsize) {
                array_set_multi(data, bitsize, 0, ALL, false);
            } else {
                const Size chunks = count / BITS;
                const Size offset = count - (chunks * BITS);
                T*       p_out = data;
                const T* p_in  = data + chunks;
                const T* p_end = data + (bitsize / BITS);
                if (offset > 0) {
                    // Shift with offset
                    const Size rbits = BITS - offset;
                    const T* p_shift_end = p_end - chunks - 1;
                    while (p_out < p_shift_end) {
                        *p_out = (*p_in << offset) | (p_in[1] >> rbits);
                        ++p_out;
                        ++p_in;
                    }
                    *p_out = (*p_in << offset);
                    ++p_out;
                } else {
                    // Shift full chunks
                    const T* p_shift_end = p_end - chunks;
                    while (p_out < p_shift_end) {
                        *p_out = *p_in;
                        ++p_out;
                        ++p_in;
                    }
                }

                // Clear remaining chunks
                for (; p_out < p_end; ++p_out)
                    *p_out = 0;
            }
        }
    }

    /** Shift to the right in chunked bit array.
     - New vacated bits on the left are zero-filled
     .
     \param  data     Bit array data pointer
     \param  bitsize  Number of bits in bit array
     \param  count    Count to shift
    */
    static void array_shiftr(T* data, Size bitsize, uint count) {
        assert( !IntegerT<T>::SIGN );
        if (bitsize > 0) {
            if (count >= bitsize) {
                array_set_multi(data, bitsize, 0, ALL, false);
            } else {
                const Size chunks = count / BITS;
                const Size offset = count - (chunks * BITS);
                T*       p_out = data + ((bitsize + (BITS-1)) / BITS);;
                const T* p_in  = p_out - chunks - 1;
                if (offset > 0) {
                    // Shift with offset
                    const Size lbits = BITS - offset;
                    const T* p_shift_end = data + chunks + 1;
                    const Size partial_tail = bitsize % BITS;
                    if (partial_tail > 0) {
                        // Mask out unused bits at end
                        if (p_out > p_shift_end) {
                            --p_in;
                            *--p_out = ((*p_in << lbits) | (p_in[1] >> offset)) & ~(ALLBITS >> partial_tail);
                        } else {
                            *--p_out = (*p_in >> offset) & ~(ALLBITS >> partial_tail);
                            return;
                        }
                    }
                    while (p_out > p_shift_end) {
                        --p_in;
                        *--p_out = (*p_in << lbits) | (p_in[1] >> offset);
                    }
                    *--p_out = (*p_in >> offset);
                } else {
                    // Shift full chunks
                    const T* p_shift_end = data + chunks;
                    while (p_out > p_shift_end) {
                        *--p_out = *p_in;
                        --p_in;
                    }
                }

                // Clear remaining chunks
                while (p_out > data)
                    *--p_out = 0;
            }
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif
#endif
