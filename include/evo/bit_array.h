// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file bit_array.h Evo BitArray. */
#pragma once
#ifndef INCL_evo_bit_array_h
#define INCL_evo_bit_array_h

#include "bits.h"
#include "impl/iter.h"
#include "impl/str.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** A subset of a BitArray.
 - This references a parent BitArrayT (or is null) and has a similar interface to BitArray
 - %Bits are positioned left-to-right, as if from an array of bools
 - Modifying bits in this subset actually modifies the parent BitArray
 - This maintains const-correctness so writes will fail if referencing a const parent
 - \b Caution: Resizing, setting, or freeing the parent will invalidate all subsets referencing it
 .

C++11:
 - Move semantics

Note that this is not a full EvoContainer and doesn't have any iterators.

 - See BitArrayT notes on performance.
 - For default parent use \ref BitArraySubset.

\par Constructors
 - BitArraySubsetT()
 - BitArraySubsetT(const Parent&)
 - BitArraySubsetT(Parent&)
 - BitArraySubsetT(const Parent&,Size,Size)
 - BitArraySubsetT(Parent&,Size,Size)
 - BitArraySubsetT(const ThisType&)
 - BitArraySubsetT(ThisType&&)
 .

\par Read Access

 - size()
   - null(), empty()
   - readonly(), offset()
   - parent() const, parent_nonconst()
 - getbit()
   - checkall()
   - checkany()
   - countbits()
 - extractl(), extractr()
 - format()
 - operator[]()
 .

\par Modifiers

 - setbit()
   - clearbit()
   - togglebit()
   - store()
 - setbit(), setbits()
   - clearbit(), clearbits()
   - togglebit(), togglebits()
 - store()
 - set()
   - set(const ThisType&,Size,Size)
   - set(const Parent&,Size,Size)
   - set(Parent&,Size,Size)
   - operator=(const ThisType&)
   - operator=(ThisType&&)
   - operator=(const Parent&)
   - operator=(Parent&)
 .

\par Example

\code
#include <evo/bit_array.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create 10-bit array 
    BitArray array(10);

    // Set first and last bits
    array.setbit(0);
    array.setbit(9);

    // Create subset excluding first and last bits
    BitArraySubset subset(array, 1, 8);

    // Set all bits 2-6 in subset, setting bits 3-7 in parent bit array
    subset.setbits(2, 5);

    // Extract and print integer in hex from all 8 bits in subset: 00111110
    uint16 val = subset.extractr<uint16>(0, 8);
    c.out << FmtUInt16(val, fHEX) << NL;

    // Extract and print integer in hex from all 10 bits: 1001111101
    val = array.extractr<uint16>(0, 10);
    c.out << FmtUInt16(val, fHEX) << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
3E
27D
\endcode
*/
template<class TParent>
class BitArraySubsetT {
public:
    typedef BitArraySubsetT<TParent> ThisType;  ///< This BitArraySubset type
    typedef TParent Parent;                     ///< Parent BitArray type
    typedef typename TParent::Size  Size;       ///< Size integer type
    typedef typename TParent::Value Value;      ///< Chunk value type

    /** Constructor sets as null. */
    BitArraySubsetT() : parent_rd_(NULL), parent_wr_(NULL), offset_(0), bitsize_(0) {
    }

    /** Constructor to reference a parent BitArray as read-only.
     \param  parent  Parent to reference (const)
    */
    BitArraySubsetT(const Parent& parent) : parent_rd_(&parent), parent_wr_(NULL), offset_(0), bitsize_(parent.bitsize_) {
    }

    /** Constructor to reference a parent BitArray.
     \param  parent  Parent to reference
    */
    BitArraySubsetT(Parent& parent) : parent_rd_(&parent), parent_wr_(&parent), offset_(0), bitsize_(parent.bitsize_) {
    }

    /** Constructor to reference a subset of parent BitArray as read-only.
     \param  parent  Parent to reference (const)
     \param  pos     Bit posiiton in parent for subset
     \param  count   Number of bits in parent for subset
    */
    BitArraySubsetT(const Parent& parent, Size pos, Size count=ALL) {
        set(parent, pos, count);
    }

    /** Constructor to reference a subset of parent BitArray.
     \param  parent  Parent to reference
     \param  pos     Bit posiiton in parent for subset
     \param  count   Number of bits in parent for subset
    */
    BitArraySubsetT(Parent& parent, Size pos, Size count=ALL) {
        set(parent, pos, count);
    }

    /** Copy constructor to reference the same parent and subset.
     \param  src  Subset to copy from
    */
    BitArraySubsetT(const ThisType& src) : parent_rd_(src.parent_rd_), parent_wr_(src.parent_wr_), offset_(src.offset_), bitsize_(src.bitsize_) {
    }

#if defined(EVO_CPP11)
    /** Move constructor (C++11).
     \param  src  Source to move
    */
    BitArraySubsetT(ThisType&& src) {
        ::memcpy(this, &src, sizeof(ThisType));
        ::memset(&src, 0, sizeof(ThisType));
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    ThisType& operator=(ThisType&& src) {
        ::memcpy(this, &src, sizeof(ThisType));
        ::memset(&src, 0, sizeof(ThisType));
        return *this;
    }
#endif

    // SET

    /** Assignment operator to copy a subset, referencing the same parent.
     \param  src  Subset to copy from
     \return      This
    */
    ThisType& operator=(const ThisType& src) {
        parent_rd_ = src.parent_rd_;
        parent_wr_ = src.parent_wr_;
        offset_ = src.offset_;
        bitsize_ = src.bitsize_;
        return *this;
    }

    /** Assignment operator to reference a new parent as read-only.
     \param  parent  Parent to reference (const)
     \return         This
    */
    ThisType& operator=(const Parent& parent) {
        parent_rd_ = &parent;
        parent_wr_ = NULL;
        offset_  = 0;
        bitsize_ = parent.size();
        return *this;
    }

    /** Assignment operator to reference a new parent.
     \param  parent  Parent to reference
     \return         This
    */
    ThisType& operator=(Parent& parent) {
        parent_rd_ = parent_wr_ = &parent;
        offset_  = 0;
        bitsize_ = parent.size();
        return *this;
    }

    /** %Set as null.
     \return  This
    */
    ThisType& set() {
        parent_rd_ = parent_wr_ = NULL;
        offset_  = 0;
        bitsize_ = 0;
        return *this;
    }

    /** %Set as subset of another subset.
     \param  src    Subset to copy from
     \param  pos    Bit position for subset of subset
     \param  count  Number of bits for subset of subset
     \return        This
    */
    ThisType& set(const ThisType& src, Size pos=0, Size count=ALL) {
        parent_rd_ = src.parent_rd_;
        parent_wr_ = src.parent_wr_;
        offset_ = src.offset_ + pos;
        if (offset_ > src.bitsize_) {
            offset_ = src.bitsize_;
            bitsize_ = 0;
        } else {
            bitsize_ = src.bitsize_ - offset_;
            if (bitsize_ > count)
                bitsize_ = count;
        }
        return *this;
    }

    /** %Set as read-only subset of BitArray.
     \param  parent  Parent to reference (const)
     \param  pos     Bit posiiton in parent for subset
     \param  count   Number of bits in parent for subset
    */
    ThisType& set(const Parent& parent, Size pos=0, Size count=ALL) {
        parent_rd_ = &parent;
        parent_wr_ = NULL;
        offset_ = pos;
        if (offset_ > parent.bitsize_) {
            offset_ = parent.bitsize_;
            bitsize_ = 0;
        } else {
            bitsize_ = parent.bitsize_ - offset_;
            if (bitsize_ > count)
                bitsize_ = count;
        }
        return *this;
    }

    /** %Set as subset of BitArray.
     \param  parent  Parent to reference
     \param  pos     Bit posiiton in parent for subset
     \param  count   Number of bits in parent for subset
    */
    ThisType& set(Parent& parent, Size pos=0, Size count=ALL) {
        parent_rd_ = parent_wr_ = &parent;
        offset_ = pos;
        if (offset_ > parent.bitsize_) {
            offset_ = parent.bitsize_;
            bitsize_ = 0;
        } else {
            bitsize_ = parent.bitsize_ - offset_;
            if (bitsize_ > count)
                bitsize_ = count;
        }
        return *this;
    }

    // INFO

    /** Get whether null.
     - Always empty when null
     .
     \return  Whether null
    */
    bool null() const {
        return (parent_rd_ == NULL);
    }

    /** Get whether empty.
     - Empty when size() is 0
     .
     \return  Whether empty
    */
    bool empty() const {
        return (bitsize_ == 0);
    }

    /** Get size as number of bits in subset.
     \return  Subset size, 0 if empty
    */
    Size size() const {
        return bitsize_;
    }

    /** Get subset offset position in parent.
     \return  Offset position
    */
    Size offset() const {
        return offset_;
    }

    /** Get whether subset is read-only, meaning writes will fail.
     - Null is also considered read-only
     .
     \return  Whether read-only
    */
    bool readonly() const {
        return (parent_wr_ == NULL);
    }

    /** Get pointer to parent BitArray.
     \return  Parent pointer, NULL if null
    */
    const Parent* parent() const {
        return parent_rd_;
    }

    /** Get non-const pointer to parent BitArray.
     \return  Parent pointer, NULL if read-only or null
    */
    Parent* parent_nonconst() {
        return parent_wr_;
    }

    // BITS

    /** Get bit at position in subset (const).
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos  Bit position in subset to get, 0 for first bit on left, etc
     \return      Bit value, false if `pos` is out of subset bounds
     \see getbit(), setbit()
    */
    bool operator[](Size pos) const {
        return (parent_rd_ != NULL && pos < bitsize_ && Bits<Value,Size>::array_get(parent_rd_->data_, parent_rd_->bitsize_, offset_ + pos));
    }

    /** Count number of bits set or cleared in subset (const).
     - This scans through all bits in subset and counts the number of bits set or cleared
     - For best performace see notes for bits_popcount()
     .
     \param  value  Whether to count bits set, false to count cleared bits (not set)
     \return        Number of bits set or cleared
    */
    Size countbits(bool value=true) const {
        if (parent_rd_ == NULL)
            return 0;
        if (value)
            return Bits<Value,Size>::array_countbits(parent_rd_->data_, parent_rd_->bitsize_, offset_, bitsize_);
        return bitsize_ - Bits<Value,Size>::array_countbits(parent_rd_->data_, parent_rd_->bitsize_, offset_, bitsize_);
    }

    /** Check if all bits are set in subset (const).
     - This scans and checks that all bits are set in subset, and stops early if a bit isn't set
     .
     \return  Whether all bits are set, true if empty
    */
    Size checkall() const {
        return (parent_rd_ == NULL || Bits<Value,Size>::array_checkall(parent_rd_->data_, parent_rd_->bitsize_, offset_, bitsize_));
    }

    /** Check if any bits are set in subset (const).
     - This scans and checks that at least 1 bit is set in subset, and stops early if a bit is set
     .
     \return  Whether at least 1 bit is set, false if all bits are cleared or if empty
    */
    bool checkany() const {
        return (parent_rd_ != NULL && Bits<Value,Size>::array_checkany(parent_rd_->data_, parent_rd_->bitsize_, offset_, bitsize_));
    }

    /** Get bit at position in subset (const).
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos  Bit position in subset to get, 0 for first bit on left, etc
     \return      Bit value, false if `pos` is out of bounds
     \see setbit(), clearbit(), togglebit()
    */
    bool getbit(Size pos) const {
        return (parent_rd_ != NULL && pos < bitsize_ && Bits<Value,Size>::array_get(parent_rd_->data_, parent_rd_->bitsize_, offset_ + pos));
    }

    /** %Set or clear bit at position in subset.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position in subset to set, 0 for first bit on left, etc
     \param  value    Bit value to set, true to set bit, false to clear bit
     \return          Whether successful, false if `pos` out of bounds
     \see setbits(), clearbit(), togglebit(), getbit()
    */
    bool setbit(Size pos, bool value=true) {
        return (parent_wr_ != NULL && pos < bitsize_ && Bits<Value,Size>::array_set(parent_wr_->data_, parent_wr_->bitsize_, offset_ + pos, value));
    }

    /** %Set or clear count bits at position in subset.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position in subset to set, 0 for first bit on left, etc
     \param  count    Number of bits to set from pos, ALL for all from pos in subset
     \param  value    Bit value to set, true to set bits, false to clear bits
     \return          Number of bits modified, 0 if out of bounds or count=0
     \see setbit(), clearbits(), togglebits()
    */
    Size setbits(Size pos=0, Size count=ALL, bool value=true) {
        if (parent_wr_ != NULL && pos < bitsize_) {
            const Size maxcount = bitsize_ - pos;
            if (count > maxcount)
                count = maxcount;
            return Bits<Value,Size>::array_set_multi(parent_wr_->data_, parent_wr_->bitsize_, offset_ + pos, count, value);
        }
        return 0;
    }

    /** Clear bit at position in subset.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos  Bit position in subset to clear, 0 for first bit on left, etc
     \return      Whether successful, false if `pos` out of bounds
     \see clearbits(), setbit(), togglebit(), getbit()
    */
    bool clearbit(Size pos) {
        return (parent_wr_ != NULL && pos < bitsize_ && Bits<Value,Size>::array_set(parent_wr_->data_, parent_wr_->bitsize_, offset_ + pos, false));
    }

    /** Clear count bits at position in subset.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos    Bit position in subset to clear, 0 for first bit on left, etc
     \param  count  Number of bits to clear from pos, ALL for all from pos in subset
     \return        Number of bits modified, 0 if out of bounds or count=0
     \see clearbit(), setbits(), togglebits()
    */
    Size clearbits(Size pos=0, Size count=ALL) const {
        if (parent_wr_ != NULL && pos < bitsize_) {
            const Size maxcount = bitsize_ - pos;
            if (count > maxcount)
                count = maxcount;
            return Bits<Value,Size>::array_set_multi(parent_wr_->data_, parent_wr_->bitsize_, offset_ + pos, count, false);
        }
        return 0;
    }

    /** Toggle bit at position in subset.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position in subset to toggle, 0 for first bit on left, etc
     \return          Whether successful, false if `pos` out of bounds
     \see togglebits(), setbit(), getbit()
    */
    bool togglebit(Size pos) {
        return (parent_wr_ != NULL && pos < bitsize_ && Bits<Value,Size>::array_toggle(parent_wr_->data_, parent_wr_->bitsize_, offset_ + pos));
    }

    /** Toggle count bits at position in subset.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position in subset to toggle, 0 for first bit on left, etc
     \param  count    Number of bits to toggle from pos, ALL for all from pos in subset
     \return          Number of bits modified, 0 if out of bounds or count=0
     \see togglebit(), setbits()
    */
    Size togglebits(Size pos=0, uint count=ALL) {
        if (parent_wr_ != NULL && pos < bitsize_) {
            const Size maxcount = bitsize_ - pos;
            if (count > maxcount)
                count = maxcount;
            return Bits<Value,Size>::array_toggle_multi(parent_wr_->data_, parent_wr_->bitsize_, offset_ + pos, count);
        }
        return 0;
    }

    /** Stores bits from value in subset.
     - %Bits are stored in chunks but are positioned left-to-right, as if in an array of bools
     - This works the same regardless of chunk size, or system endianness
     - Value bits are taken from the right so integers can be stored
     - Example value `00000111` with `count=3` would store `111` (3 right-most bits) or `7` in decimal
     - %Bits stored after end of subset are truncated and lost
     - \b Caution: If count exceeds `value` size, count is capped at that size -- only bits from `value` are stored
     .
     \tparam  U  Value type to store, signed value is treated as unsigned -- inferred from `value` param
     \param  pos      Bit position in subset to store at, 0 for first bit on left, etc
     \param  count    Number of bits to store from value
     \param  value    Value to store (`count` right-most bits are stored)
     \return          Whether successful, false if `pos` is out of bounds
     \see setbit(), extractl(), extractr()
    */
    template<class U>
    bool store(Size pos, Size count, U value) {
        const Size maxcount = bitsize_ - pos;
        if (count > maxcount)
            count = maxcount;
        return (parent_wr_ != NULL && pos < bitsize_ && Bits<Value,Size>::array_store(parent_wr_->data_, parent_wr_->bitsize_, offset_ + pos, count, value));
    }

    /** Extract bits from subset.
     - %Bits are stored in chunks but are extracted left-to-right, as if from an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Extracted bits are left-aligned in the return value, and the left-most bit is the "first" bit (as if from an array of bools)
       - Example with 3 extracted bits `111` returned as `uint8`: `11100000`
     .
     \tparam  U  Return type for extracted bits, must be unsigned (uint, ulong, uint32, etc) -- defaults to `ulong` in C++11 or newer
     \param  pos    Bit position to extract from, 0 for first bit on left, etc
     \param  count  Number of bits to extract, truncated if larger than bitsize or return value
     \return        Extracted bits, aligned to the left
     \see extractr(), get(), store()
    */
    template<class U EVO_ONCPP11(=ulong)>
    U extractl(Size pos=0, Size count=ALL) const {
        if (parent_rd_ != NULL && pos < bitsize_) {
            const Size maxcount = bitsize_ - pos;
            if (count > maxcount)
                count = maxcount;
            return Bits<Value,Size>::template array_extractl<U>(parent_rd_->data_, parent_rd_->bitsize_, offset_ + pos, count);
        }
        return 0;
    }

    /** Extract bits from subset.
     - %Bits are stored in chunks but are extracted left-to-right, as if from an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Extracted bits returned are right-aligned so the result is an integer
       - Example with 3 extracted bits `111` returned as `uint8`: `00000111` or `7` in decimal
     - A `count` larger than bits in return type will be truncated -- _this should be avoided_ as it may not be easy to tell how many bits were truncated
       - For example, if you pass `count=9` with return type `uint8` then this uses `count=8`
     - Otherwise `count` may go out of bit array bounds without truncation -- bits from out of bounds are set to 0 and still are right-aligned according to `count`
     .
     \tparam  U  Return type for extracted bits, must be unsigned (uint, ulong, uint32, etc) -- defaults to `ulong` in C++11 or newer
     \param  pos    Bit position to extract from, 0 for first bit on left, etc
     \param  count  Number of bits to extract, ALL for num bits in return type, truncated if larger than bits in return type
     \return        Extracted bits, aligned to the right
     \see extractl(), get(), store()
    */
    template<class U EVO_ONCPP11(=ulong)>
    U extractr(Size pos=0, Size count=ALL) const {
        if (parent_rd_ != NULL && pos < bitsize_) {
            const Size maxcount = bitsize_ - pos;
            if (count > maxcount)
                count = maxcount;
            return Bits<Value,Size>::template array_extractr<U>(parent_rd_->data_, parent_rd_->bitsize_, offset_ + pos, count);
        }
        return 0;
    }

    /** Format bits from subset to stream or string using base.
     - If there aren't enough bits for the last digit then zero-bits are padded on the right to finish, ex: 1 set bit formatted as hex is `8`, or 5 set bits formatted as hex is `F8`
       - %Bits are ordered left-to-right, as if from an array of bools, so this padding is added at the end
     .
     \tparam  U  Output string/stream type, deduced from arguments
     \param  out   Output string or stream (or associated Format object) to format to
     \param  base  Base to format as, must be a power of 2 between 2-32 (inclusive), add 100 for lowercase letters, or use enum:
                    - fBIN for binary
                    - fOCT for octal
                    - fHEX for hexidecimal
                    - fHEXL for lowercase hexadecimal
     \return       Whether successful, false on stream error or if base is invalid
    */
    template<class U>
    bool format(U& out, int base=fBIN) {
        if (bitsize_ > 0) {
            assert( parent_rd_ != NULL );
            const char* digits;
            if (base >= 100) {
                base -= 100;
                digits = "0123456789abcdefghijklmnopqrstuvwxyz";
            } else
                digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            int bits_per_digit;
            switch (base) {
                case 2:  bits_per_digit = 1; break;
                case 4:  bits_per_digit = 2; break;
                case 8:  bits_per_digit = 3; break;
                case 16: bits_per_digit = 4; break;
                case 32: bits_per_digit = 5; break;
                default: return false;
            }

            typename U::Out& outstream = out.write_out();
            typename U::Out::Size available = 0;

            Size outsize = (bitsize_ + bits_per_digit - 1) / bits_per_digit; // round up
            char* outbuf = outstream.write_direct_multi(available, outsize);
            if (outbuf == NULL)
                return false;

            char* outbuf_start = outbuf;
            char* outbuf_end   = outbuf + available;
            char* outbuf_write;
            char* outbuf_next;
            Size outsize_lastwrite = outsize;
            Size digit_count;

            const Value* parent_data    = parent_rd_->data_;    // local copy of data pointer
            const Size   parent_bitsize = offset_ + bitsize_;   // use reduced parent bitsize to exclude ending partial digit bits from extractr()

            // Use extractr() to extract and format bits
            const uint digits_per_ulong = IntegerT<ulong>::BITS / bits_per_digit;
            const uint bits_per_ulong   = digits_per_ulong * bits_per_digit;
            ulong num;
            outbuf_end -= digits_per_ulong;
            for (Size p=offset_; outsize > 0; p += bits_per_ulong) {
                if (outbuf > outbuf_end) {
                    // No room, flush buffer
                    outbuf_start = outbuf = outstream.write_direct_flush(available, (ulong)(outbuf - outbuf_start), outsize);
                    if (outbuf == NULL)
                        return false;
                    outbuf_end = outbuf + available - digits_per_ulong;
                    outsize_lastwrite = outsize;
                }

                if (outsize < digits_per_ulong)
                    digit_count = outsize;
                else
                    digit_count = digits_per_ulong;
                num = Bits<Value,Size>::template array_extractr<ulong>(parent_data, parent_bitsize, p, digit_count * bits_per_digit);

                // Format chunk
                outbuf_next = outbuf_write = outbuf + digit_count;
                for (; num != 0; num /= (Value)base)
                    *--outbuf_write = digits[num % base];
                while (outbuf_write > outbuf)
                    *--outbuf_write = '0';
                outsize -= digit_count;
                outbuf = outbuf_next;
            }

            assert( outsize_lastwrite >= outsize );
            outstream.write_direct_finish(outsize_lastwrite - outsize);
        }
        return true;
    }

    // COMPARE

    /** Equality operator.
     \param  data  Subset to compare to
     \return       Whether equal
    */
    bool operator==(const ThisType& data) const {
        bool result;
        if (this == &data)
            result = true;
        else if (parent_rd_ == NULL)
            result = (data.parent_rd_ == NULL);
        else if (data.parent_rd_ == NULL || bitsize_ != data.bitsize_)
            result = false;
        else if (bitsize_ == 0)
            result = true;
        else {
            const Size   this_index   = offset_ / Bits<Value,Size>::BITS;
            const Size   this_offset  = offset_ - (this_index * Bits<Value,Size>::BITS);
            const Value* this_ptr     = parent_rd_->data_ + this_index;
            const Size   other_index  = data.offset_ / Bits<Value,Size>::BITS;
            const Size   other_offset = data.offset_ - (other_index * Bits<Value,Size>::BITS);
            Size count = bitsize_;
            Value maskval;
            if (this_offset == other_offset) {
                // Easier to compare with same offsets
                const Value* other_ptr = data.parent_rd_->data_ + other_index;

                // Leading partial chunk
                if (this_offset > 0) {
                    maskval = Bits<Value,Size>::ALLBITS >> this_offset;
                    if ((*this_ptr & maskval) != (*other_ptr & maskval))
                        return false;
                    count -= (Bits<Value, Size>::BITS - this_offset);
                    ++this_ptr;
                    ++other_ptr;
                }

                // Compare full chunks
                while (count >= Bits<Value, Size>::BITS) {
                    count -= Bits<Value, Size>::BITS;
                    if (*this_ptr != *other_ptr)
                        return false;
                    ++this_ptr;
                    ++other_ptr;
                }

                // Compare trailing partial chunk
                if (count > 0) {
                    maskval = ~(Bits<Value,Size>::ALLBITS >> count);
                    if ((*this_ptr & maskval) != (*other_ptr & maskval))
                        return false;
                }
            } else {
                // Trickier to compare with different offsets, use array_extractl() for other data
                const Value* other_ptr   = data.parent_rd_->data_;
                const Size other_bitsize = data.parent_rd_->bitsize_;
                Size       other_pos     = data.offset_;

                // Leading partial chunk
                if (this_offset > 0) {
                    maskval = Bits<Value,Size>::ALLBITS >> this_offset;
                    if ( (*this_ptr & maskval) != Bits<Value,Size>::template array_extractr<Value>(other_ptr, other_bitsize, other_pos, (Bits<Value,Size>::BITS - this_offset)) )
                        return false;
                    ++this_ptr;
                    const Size bits = Bits<Value, Size>::BITS - this_offset;
                    count -= bits;
                    other_pos += bits;
                }

                // Compare full chunks
                while (count >= Bits<Value, Size>::BITS) {
                    count -= Bits<Value, Size>::BITS;
                    if (*this_ptr != Bits<Value,Size>::template array_extractl<Value>(other_ptr, other_bitsize, other_pos, Bits<Value,Size>::BITS))
                        return false;
                    ++this_ptr;
                    other_pos += Bits<Value, Size>::BITS;
                }

                // Compare trailing partial chunk
                if (count > 0) {
                    maskval = ~(Bits<Value,Size>::ALLBITS >> count);
                    if ((*this_ptr & maskval) != Bits<Value,Size>::template array_extractl<Value>(other_ptr, other_bitsize, other_pos, count))
                        return false;
                }
            }
            result = true;
        }
        return result;
    }

    /** Inequality operator.
     \param  data  Subset to compare to
     \return       Whether inequal
    */
    bool operator!=(const ThisType& data) const {
        return !operator==(data);
    }

private:
    const Parent* parent_rd_;   // reads
    Parent* parent_wr_;         // writes
    Size offset_;
    Size bitsize_;
};

///////////////////////////////////////////////////////////////////////////////

/** Dynamic bit array container with similar interface to Array and List.

\tparam  T      Chunk value type for storing bits, must be an unsigned integer type -- default: ulong
\tparam  TSize  Size integer type, must be unsigned -- default: SizeT

\par Features

 - %Bits are stored internally as array of chunk values
 - %Bits are positioned left-to-right, as if from an array of bools
 - Chunk values are stored sequentially in contiguous memory -- random access uses constant time
 - No memory allocated by new empty bit array
 - No extra capacity allocation, sharing, or slicing like List
 .

C++11:
 - Range-based for loop
   \code
    BitArray arr;
    for (auto bit : arr) {
    }
   \endcode
 - Initialization lists (uint32 list)
   \code
    BitArray arr = {0xFFFFFFFF, 0x00000000};
   \endcode
 - Move semantics
   - \b Caution: Moving a BitArray invalidates any BitArraySubetT referencing it

Note that this is not a full EvoContainer and has more limited iterators.

 - For subset see BitArraySubsetT
 - For default size types use \ref BitArray and \ref BitArraySubset

\par Constructors

 - BitArrayT()
 - BitArrayT(Size)
 - BitArrayT(const ThisType&)
 - BitArrayT(const ThisType&,Size,Size)
 - BitArrayT(const Subset&,Size,Size)
 - BitArrayT(std::initializer_list<uint32>) [C++11]
 - BitArrayT(ThisType&&) [C++11]
 .

\par Read Access

 - asconst()
 - size()
   - null(), empty()
   - shared()
 - cbegin(), cend()
   - begin(), end()
 - getbit()
   - checkall()
   - checkany()
   - countbits()
 - extractl(), extractr()
 - format()
 - data() const
   - operator[](Size) const
   - hash()
 - operator==()
   - operator!=()
 .

\par Modifiers

 - setbit()
   - clearbit()
   - togglebit()
   - store()
 - data()
 - resize()
   - resize_pow2()
   - unshare()
 - setbit(), setbits()
   - clearbit(), clearbits()
   - togglebit(), togglebits()
 - store()
 - load()
 - clear()
 - set()
   - set(const ThisType&)
   - set(const Subset&)
   - setempty()
   - operator=(const ThisType& data)
   - operator=(ThisType&&) [C++11]
 - shiftl(), shiftr()
 .

\par Example

\code
#include <evo/bit_array.h>
#include <evo/io.h>
using namespace evo;
static Console& c = con();

int main() {
    // Create 10-bit array 
    BitArray array(10);

    // Set first and last bits
    array.setbit(0);
    array.setbit(9);

    // Store integer 7 (111 in binary) in bits 2-4
    array.store(2, 3, 7u);

    // Get a couple individual bits
    const bool bit0 = array.getbit(0);      // true
    const bool bit1 = array.getbit(1);      // false

    // Extract and print integer from bits 2-4: 111
    c.out << array.extractr<uint16>(2, 3) << NL;

    // Extract and print integer in hex from all 10 bits: 1011100001
    const uint16 val = array.extractr<uint16>(0, 10);
    c.out << FmtUInt16(val, fHEX) << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
7
2E1
\endcode
*/
template<class T=ulong,class TSize=SizeT>
class BitArrayT {
public:
    EVO_CONTAINER_TYPE;                         ///< Identify Evo container type
    typedef BitArrayT<T,TSize> ThisType;        ///< This bit array type
    typedef BitArraySubsetT<ThisType> Subset;   ///< Subset type for this bit array type
    typedef TSize Size;                         ///< Size integer type
    typedef T     Value;                        ///< Chunk value type for bits

    // Iterator support types
    /** \cond impl */
    struct IterKey {
        Size offset;
        typename Bits<T,Size>::IterState state;
    };
    typedef Size IterItem;
    /** \endcond */

    typedef typename IteratorFw<ThisType>::Const Iter;    ///< Iterator (const) - IteratorFw

    /** Default constructor sets as null. */
    BitArrayT() : data_(NULL), size_(0), bitsize_(0) {
    }

    /** Constructor create bit array.
     \param  bitsize  Bit size to initialize with
    */
    BitArrayT(Size bitsize) : data_(NULL), size_(0), bitsize_(0) {
        resize(bitsize);
    }

    /** Copy constructor.
     \param  src  Bit array to copy
    */
    BitArrayT(const ThisType& src) {
        if (src.size_ > 0) {
            data_ = (Value*)::malloc((size_t)src.size_ * sizeof(Value));
            memcpy(data_, src.data_, src.size_ * sizeof(Value));
        } else
            data_ = src.data_;
        size_    = src.size_;
        bitsize_ = src.bitsize_;
    }

    /** Copy from another bit array.
     \param  src    Bit array subset to copy from
     \param  pos    Bit position in src to copy from, 0 for first bit on left, etc
     \param  count  Number of bits to copy from src, ALL for all from pos
    */
    BitArrayT(const ThisType& src, Size pos, Size count=ALL) : data_(NULL), size_(0), bitsize_(0) {
        if (count > src.bitsize_)
            count = src.bitsize_;
        if (count > 0) {
            resize(count);
            Bits<T,TSize>::array_copy(data_, bitsize_, src.data_, src.bitsize_, pos, count);
        }
    }

    /** Copy from subset.
     - This will resize() to count or subset size (whichever is lower), and copy
     .
     \param  subset  Bit array subset to copy from
     \param  pos     Bit position in subset to copy from, 0 for first bit on left, etc
     \param  count   Number of bits to copy from subset, ALL for all from pos
    */
    BitArrayT(const Subset& subset, Size pos=0, Size count=ALL) : data_(NULL), size_(0), bitsize_(0) {
        const ThisType* parent = subset.parent();
        if (parent != NULL) {
            const Size subset_bitsize = subset.size();
            if (count > subset_bitsize)
                count = subset_bitsize;
            if (count > 0) {
                resize(count);
                Bits<T,TSize>::array_copy(data_, bitsize_, parent->data_, subset_bitsize, subset.offset() + pos, count);
            }
        }
    }

    /** Destructor. */
    ~BitArrayT() {
        if (size_ > 0)
            ::free(data_);
    }

#if defined(EVO_CPP11)
    /** Sequence constructor that initializes bits from a list of uint32 values (C++11).
     \param  init  Initializer list, passed as comma-separated uint32 values in braces `{ }`
    */
    BitArrayT(std::initializer_list<uint32> init) : BitArrayT() {
        const Size ITEM_BITS = sizeof(uint32) * 8;
        assert( init.size() < IntegerT<Size>::MAX / ITEM_BITS );
        resize((Size)init.size() * ITEM_BITS);
        Size offset = 0;
        for (auto num : init) {
            store(offset, ITEM_BITS, num);
            offset += ITEM_BITS;
        }
    }

    /** Move constructor (C++11).
     \param  src  Source to move
    */
    BitArrayT(ThisType&& src) {
        ::memcpy(this, &src, sizeof(ThisType));
        ::memset(&src, 0, sizeof(ThisType));
    }

    /** Move assignment operator (C++11).
     \param  src  Source to move
     \return      This
    */
    ThisType& operator=(ThisType&& src) {
        resize(0);
        ::memcpy(this, &src, sizeof(ThisType));
        ::memset(&src, 0, sizeof(ThisType));
        return *this;
    }
#endif

    /** Explicitly use a const reference to this.
     - This is useful to force using this as const without casting
     .
     \return  This
    */
    const ThisType& asconst() const {
        return *this;
    }

    // SET

    /** Assignment operator.
     \param  src  Bit array to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src) {
        return set(src);
    }

    /** Clear by freeing all values.
     - Does not set as null -- null status is unchanged
     .
     \return  This
    */
    ThisType& clear() {
        if (size_ > 0) {
            ::free(data_);
            data_ = EVO_PEMPTY;
            size_ = 0;
            bitsize_ = 0;
        }
        return *this;
    }

    /** %Set as null and empty.
     \return  This
    */
    ThisType& set() {
        if (size_ > 0) {
            ::free(data_);
            size_ = 0;
            bitsize_ = 0;
        }
        data_ = NULL;
        return *this;
    }

    /** %Set as copy of another bit array.
     \param  src  Data to copy
     \return      This
    */
    ThisType& set(const ThisType& src) {
        if (this != &src) {
            if (src.size_ > 0) {
                const size_t size_bytes = (size_t)src.size_ * sizeof(Value);
                if (size_ == src.size_) {
                    // Same positive size
                    memcpy(data_, src.data_, size_bytes);
                } else {
                    // New positive size
                    if (size_ > 0)
                        ::free(data_);
                    size_ = src.size_;
                    data_ = (Value*)::malloc(size_bytes);
                    memcpy(data_, src.data_, size_bytes);
                }
                bitsize_ = src.bitsize_;
            } else {
                // Null/Empty
                if (size_ > 0) {
                    ::free(data_);
                    size_ = 0;
                }
                bitsize_ = 0;
                data_ = (src.data_ == NULL ? NULL : EVO_PEMPTY);
            }
        }
        return *this;
    }

    /** %Set as copy of bit array subset.
     - This invalidates any existing subsets referrencing to this bit array
     .
     \param  src  Data to copy, which may reference this same BitArray
     \return      This
    */
    ThisType& set(const Subset& src) {
        const ThisType* src_parent = src.parent();
        if (this == src_parent) {
            Bits<T,Size>::array_shiftl(data_, bitsize_, src.offset());
            resize(src.size());
        } else if (src_parent == NULL) {
            set();
        } else {
            const Size src_size = src.size();
            resize(src_size);
            Bits<T,TSize>::array_copy(data_, bitsize_, src_parent->data_, src_size);
        }
        return *this;
    }

    /** %Set as empty but not null.
     \return  This
    */
    ThisType& setempty() {
        if (size_ > 0) {
            ::free(data_);
            size_ = 0;
            bitsize_ = 0;
        }
        data_ = EVO_PEMPTY;
        return *this;
    }

    // INFO

    /** Get whether null.
     - Always empty when null
     .
     \return  Whether null
    */
    bool null() const {
        return (data_ == NULL);
    }

    /** Get whether empty.
     - Empty when size() is 0
     .
     \return  Whether empty
    */
    bool empty() const {
        return (bitsize_ == 0);
    }

    /** Get bit size.
     \return  Size as number of bits stored
    */
    Size size() const {
        return bitsize_;
    }

    /** Get whether shared (false).
     - This doesn't support sharing so always returns false
     .
     \return  Whether shared (always false)
    */
    bool shared() const {
        return false;
    }

    /** Get data pointer (const).
     - \b Caution: Modifying the size of the array will invalidate returned pointer
     .
     \return  Data pointer as read-only, NULL/invalid if empty
    */
    const Value* data() const {
        return data_;
    }

    /** Get bit at position in bit array (const).
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos  Bit position to get, 0 for first bit on left, etc
     \return      Bit value, false if `pos` is out of bounds
     \see getbit(), setbit()
    */
    bool operator[](Size pos) const {
        return Bits<T,Size>::array_get(data_, bitsize_, pos);
    }

    /** Get data hash value for whole bit array.
     \param  seed  Seed value for hashing multiple values, 0 if none
     \return       Hash value
    */
    ulong hash(ulong seed=0) const {
        return DataHash<T>::hash(data_, size_, seed);
    }

    // BITS

    /** \copydoc SubList::cbegin() */
    Iter cbegin() const
        { return Iter(*this); }

    /** \copydoc SubList::cend() */
    Iter cend() const
        { return Iter(); }

    /** \copydoc SubList::begin() */
    Iter begin() const
        { return Iter(*this); }

    /** \copydoc SubList::end() */
    Iter end() const
        { return Iter(); }

    /** Count number of bits set or cleared (const).
     - This scans through all bits and counts the number of bits set or cleared
     - For best performace see notes for bits_popcount()
     .
     \param  value  Whether to count bits set, false to count cleared bits (not set)
     \return        Number of bits set or cleared
    */
    Size countbits(bool value=true) const {
        if (value)
            return Bits<T,Size>::array_countbits(data_, bitsize_);
        return bitsize_ - Bits<T,Size>::array_countbits(data_, bitsize_);
    }

    /** Check if all bits are set in bit array (const).
     - This scans and checks that all bits are set, and stops early if a bit isn't set
     .
     \return  Whether all bits are set
    */
    bool checkall() const {
        return Bits<T,Size>::array_checkall(data_, bitsize_);
    }

    /** Check if any bits are set in bit array (const).
     - This scans and checks that at least 1 bit is set, and stops early if a bit is set
     .
     \return  Whether at least 1 bit is set, false if all bits are cleared
    */
    bool checkany() const {
        return Bits<T,Size>::array_checkany(data_, bitsize_);
    }

    /** Get bit at position in bit array (const).
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos  Bit position to get, 0 for first bit on left, etc
     \return      Bit value, false if `pos` is out of bounds
     \see setbit(), clearbit(), togglebit()
    */
    bool getbit(Size pos) const {
        return Bits<T,Size>::array_get(data_, bitsize_, pos);
    }

    /** %Set or clear bit at position in bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position to set, 0 for first bit on left, etc
     \param  value    Bit value to set, true to set bit, false to clear bit
     \return          Whether successful, false if `pos` out of bounds
     \see setbits(), clearbit(), togglebit(), getbit()
    */
    bool setbit(Size pos, bool value=true) {
        return Bits<T,Size>::array_set(data_, bitsize_, pos, value);
    }

    /** %Set or clear count bits at position in bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position to set, 0 for first bit on left, etc
     \param  count    Number of bits to set from pos, ALL for all from pos
     \param  value    Bit value to set, true to set bits, false to clear bits
     \return          Number of bits modified, 0 if out of bounds or count=0
     \see setbit(), clearbits(), togglebits()
    */
    Size setbits(Size pos=0, Size count=ALL, bool value=true) {
        return Bits<T,Size>::array_set_multi(data_, bitsize_, pos, count, value);
    }

    /** Clear bit at position in bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos  Bit position to clear, 0 for first bit on left, etc
     \return      Whether successful, false if `pos` out of bounds
     \see clearbits(), setbit(), togglebit(), getbit()
    */
    bool clearbit(Size pos) {
        return Bits<T,Size>::array_set(data_, bitsize_, pos, false);
    }

    /** Clear count bits at position in bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos    Bit position to clear, 0 for first bit on left, etc
     \param  count  Number of bits to clear from pos, ALL for all from pos
     \return        Number of bits modified, 0 if out of bounds or count=0
     \see clearbit(), setbits(), togglebits()
    */
    Size clearbits(Size pos=0, Size count=ALL) const {
        return Bits<T,Size>::array_set_multi(data_, bitsize_, pos, count, false);
    }

    /** Toggle bit at position in bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position to toggle, 0 for first bit on left, etc
     \return          Whether successful, false if `pos` out of bounds
     \see togglebits(), setbit(), clearbit(), getbit()
    */
    bool togglebit(Size pos) {
        return Bits<T,Size>::array_toggle(data_, bitsize_, pos);
    }

    /** Toggle count bits at position in bit array.
     - Bits are counted left-to-right, as if from an array of bools
     .
     \param  pos      Bit position to toggle, 0 for first bit on left, etc
     \param  count    Number of bits to toggle from pos, ALL for all from pos
     \return          Number of bits modified, 0 if out of bounds or count=0
     \see togglebit(), setbits(), clearbits()
    */
    Size togglebits(Size pos=0, uint count=ALL) {
        return Bits<T,Size>::array_toggle_multi(data_, bitsize_, pos, count);
    }

    /** Stores bits from value in bit array.
     - %Bits are stored in chunks but are positioned left-to-right, as if in an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Value bits are taken from the right so integers can be stored
       - Example value `00000111` with `count=3` would store `111` (3 right-most bits) or `7` in decimal
     - %Bits stored after end of bit array are truncated and lost
     - \b Caution: If count exceeds `value` size, count is capped at that size -- only bits from `value` are stored
     .
     \tparam  U  Value type to store, signed value is treated as unsigned -- inferred from `value` param
     \param  pos      Bit position to store at, 0 for first bit on left, etc
     \param  count    Number of bits to store from value
     \param  value    Value to store (`count` right-most bits are stored)
     \return          Whether successful, false if `pos` is out of bounds
     \see setbit(), extract()
    */
    template<class U>
    bool store(Size pos, Size count, U value) {
        return Bits<T,Size>::array_store(data_, bitsize_, pos, count, value);
    }

    /** Extract bits from bit array.
     - %Bits are stored in chunks but are extracted left-to-right, as if from an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Extracted bits are left-aligned in the return value, and the left-most bit is the "first" bit (as if from an array of bools)
       - Example with 3 extracted bits `111` returned as `uint8`: `11100000`
     .
     \tparam  U  Return type for extracted bits, must be unsigned (uint, ulong, uint32, etc) -- defaults to `ulong` in C++11 or newer
     \param  pos    Bit position to extract from, 0 for first bit on left, etc
     \param  count  Number of bits to extract, truncated if larger than bitsize or return value
     \return        Extracted bits, aligned to the left
     \see extractr(), get(), store()
    */
    template<class U EVO_ONCPP11(=uint32)>
    U extractl(Size pos=0, Size count=ALL) const {
        return Bits<T,Size>::template array_extractl<U>(data_, bitsize_, pos, count);
    }

    /** Extract bits from bit array.
     - %Bits are stored in chunks but are extracted left-to-right, as if from an array of bools
       - This works the same regardless of chunk size, or system endianness
     - Extracted bits returned are right-aligned so the result is an integer
       - Example with 3 extracted bits `111` returned as `uint8`: `00000111` or `7` in decimal
     - A `count` larger than bits in return type will be truncated -- _this should be avoided_ as it may not be easy to tell how many bits were truncated
       - For example, if you pass `count=9` with return type `uint8` then this uses `count=8`
     - Otherwise `count` may go out of bit array bounds without truncation -- bits from out of bounds are set to 0 and still are right-aligned according to `count`
     .
     \tparam  U  Return type for extracted bits, must be unsigned (uint, ulong, uint32, etc) -- defaults to `ulong` in C++11 or newer
     \param  pos    Bit position to extract from, 0 for first bit on left, etc
     \param  count  Number of bits to extract, ALL for num bits in return type, truncated if larger than bits in return type
     \return        Extracted bits, aligned to the right
     \see extractl(), get(), store()
    */
    template<class U EVO_ONCPP11(=uint32)>
    U extractr(Size pos=0, Size count=ALL) const {
        return Bits<T,Size>::template array_extractr<U>(data_, bitsize_, pos, count);
    }

    /** Shift all bits in bit bit array to the left.
     - New vacated bits on the right are zero-filled
     .
     \param  count  Count to shift
    */
    ThisType& shiftl(uint count) {
        Bits<T,Size>::array_shiftl(data_, bitsize_, count);
        return *this;
    }

    /** Shift all bits in bit bit array to the right.
     - New vacated bits on the left are zero-filled
     .
     \param  count  Count to shift
    */
    ThisType& shiftr(uint count) {
        Bits<T,Size>::array_shiftr(data_, bitsize_, count);
        return *this;
    }

    /** Parse and load bits from numeric string.
     - This resizes the bit array if needed to match the input string, current data is lost
     - Leading and trailing whitespace (spaces and tabs) in input string are ignored
     - This stops and returns 0 on invalid base or if an invalid digit is found
     - On error any bit values should be considered uninitialized
     - Call format() to save/format bits
     .
     \param  str   Pointer to string to load
     \param  size  %String length to load
     \param  base  %String number base to use: 2 for binary, 8 for octal, 16 for hex -- must be a power of 2 in the range 2-32 (inclusive)
     \return       Number of bits loaded, 0 on error
    */
    Size load(const char* str, Size size, int base=2) {
        // Use T or ulong, whichever is larger
        typedef typename StaticIf<(sizeof(T) > sizeof(ulong)), T, ulong>::Type TNum;

        int bits_per_digit;
        switch (base) {
            case 2:  bits_per_digit = 1; break;
            case 4:  bits_per_digit = 2; break;
            case 8:  bits_per_digit = 3; break;
            case 16: bits_per_digit = 4; break;
            case 32: bits_per_digit = 5; break;
            default: return 0;
        }

        uchar ch;
        while (size > 0 && ((ch=str[size-1]) == ' ' || ch == '\t'))
            --size;
        const char* end = str + size;
        while (str < end && ((ch=*str) == ' ' || ch == '\t'))
            ++str;
        if (str >= end)
            return 0;

        size = (Size)(end - str) * bits_per_digit; // size is now in bits
        resize(size);

        uint bits = 0;
        TNum num  = 0;
        T* p = data_;
        for (; str < end; ++str) {
            ch = *str;
            if (ch >= '0' && ch <= '9')
                ch -= '0';
            else if (ch >= 'A' && ch <= 'V')
                ch = ch - 'A' + 10;
            else if (ch >= 'a' && ch <= 'v')
                ch = ch - 'a' + 10;
            else
                return 0; // not a digit
            if (ch >= base)
                return 0;

            bits += bits_per_digit;
            if (bits == Bits<T, Size>::BITS) {
                // No overflow
                *p = T((num * (uint)base) + ch);
                ++p;
                num = 0;
                bits = 0;
            } else if (bits > Bits<T,Size>::BITS) {
                // Protect from overflow from extra bits
                const uint shift = Bits<T,Size>::BITS - (bits - bits_per_digit);
                bits -= Bits<T,Size>::BITS;
                *p = T(num << shift) | (T(ch) >> bits);
                ++p;
                num = T(ch) & ~(Bits<T,Size>::ALLBITS << bits);
            } else {
                num *= (uint)base;
                num += ch;
            }
        }

        if (bits > 0)
            *p = T(num) << (Bits<T,Size>::BITS - bits);
        return size;
    }

    /** Format bits to stream or string using base.
     - If there aren't enough bits for the last digit then zero-bits are padded on the right to finish, ex: 1 set bit formatted as hex is `8`, or 5 set bits formatted as hex is `F8`
       - %Bits are ordered left-to-right, as if from an array of bools, so this padding is added at the end
     - Call load() to load formatted bits (must use same base)
     .
     \tparam  U  Output string/stream type, deduced from arguments
     \param  out   Output string or stream (or associated Format object) to format to
     \param  base  Base to format as, must be a power of 2 between 2-32 (inclusive), add 100 for lowercase letters, or use enum:
                    - fBIN for binary
                    - fOCT for octal
                    - fHEX for hexidecimal
                    - fHEXL for lowercase hexadecimal
     \return       Whether successful, false on stream error or if base is invalid
    */
    template<class U>
    bool format(U& out, int base=fBIN) {
        if (bitsize_ > 0) {
            const char* digits;
            if (base >= 100) {
                base -= 100;
                digits = "0123456789abcdefghijklmnopqrstuvwxyz";
            } else
                digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            int bits_per_digit;
            switch (base) {
                case 2:  bits_per_digit = 1; break;
                case 4:  bits_per_digit = 2; break;
                case 8:  bits_per_digit = 3; break;
                case 16: bits_per_digit = 4; break;
                case 32: bits_per_digit = 5; break;
                default: return false;
            }

            typename U::Out& outstream = out.write_out();
            typename U::Out::Size available = 0;

            Size outsize = (bitsize_ + bits_per_digit - 1) / bits_per_digit; // round up
            char* outbuf = outstream.write_direct_multi(available, outsize);
            if (outbuf == NULL)
                return false;

            char* outbuf_start = outbuf;
            char* outbuf_end   = outbuf + available;
            char* outbuf_write;
            char* outbuf_next;
            Size outsize_lastwrite = outsize, digit_count;

            if (Bits<T,Size>::BITS % bits_per_digit != 0) {
                // Use extractr() when bits per digit is not a multiple of bits per chunk
                const uint digits_per_ulong = IntegerT<ulong>::BITS / bits_per_digit;
                const uint bits_per_ulong   = digits_per_ulong * bits_per_digit;
                ulong num;
                outbuf_end -= digits_per_ulong;
                for (Size p=0; outsize > 0; p += bits_per_ulong) {
                    digit_count = (outsize > digits_per_ulong ? digits_per_ulong : outsize);
                    num = Bits<T,Size>::template array_extractr<ulong>(data_, bitsize_, p, digit_count * bits_per_digit);
                    if (outbuf >= outbuf_end) {
                        // No room, flush buffer
                        outbuf_start = outbuf = outstream.write_direct_flush(available, (ulong)(outbuf - outbuf_start), outsize);
                        if (outbuf == NULL)
                            return false;
                        outbuf_end = outbuf + available - digits_per_ulong;
                        outsize_lastwrite = outsize;
                    }

                    // Format chunk
                    outbuf_next = outbuf_write = outbuf + digit_count;
                    for (; num != 0; num /= (T)base)
                        *--outbuf_write = digits[num % base];
                    while (outbuf_write > outbuf)
                        *--outbuf_write = '0';
                    outsize -= digit_count;
                    outbuf = outbuf_next;
                }
            } else {
                // Easier when bits per digit is a multiple of bits per chunk
                const uint digits_per_chunk = Bits<T,Size>::BITS / bits_per_digit;
                const T* p = data_;
                const T* p_end_rndup = data_ + ((bitsize_ + Bits<T,Size>::BITS - 1) / Bits<T,Size>::BITS);  // includes partial chunk at end
                const T* p_end       = data_ + (bitsize_ / Bits<T,Size>::BITS);                             // not including partial chunk at end
                T num;

                while (outsize > 0) {
                    if (outsize < digits_per_chunk)
                        digit_count = outsize;
                    else
                        digit_count = digits_per_chunk;

                    outbuf_next = outbuf + digit_count;
                    if (outbuf_next > outbuf_end && outbuf > outbuf_start) {
                        // No room, flush buffer
                        outbuf_start = outbuf = outstream.write_direct_flush(available, (ulong)(outbuf - outbuf_start), outsize);
                        if (outbuf == NULL)
                            return false;
                        outbuf_end  = outbuf + available;
                        outbuf_next = outbuf + digit_count;
                        outsize_lastwrite = outsize;
                    }

                    // Format chunk
                    outbuf_write = outbuf_next;
                    if (p < p_end_rndup) {
                        num = *p;
                        if (p == p_end)
                            num >>= Bits<T,Size>::BITS - (outsize * bits_per_digit);
                        for (; num != 0; num /= (T)base)
                            *--outbuf_write = digits[num % base];
                        ++p;
                    }
                    while (outbuf_write > outbuf)
                        *--outbuf_write = '0';
                    assert( outbuf_next > outbuf );
                    outsize -= digit_count;
                    outbuf = outbuf_next;
                }
            }

            assert( outsize_lastwrite >= outsize );
            outstream.write_direct_finish(outsize_lastwrite - outsize);
        }
        return true;
    }

    // COMPARE

    /** Equality operator.
     \param  data  Data to compare to
     \return       Whether equal
    */
    bool operator==(const ThisType& data) const {
        bool result;
        if (this == &data)
            result = true;
        else if (data_ == NULL)
            result = (data.data_ == NULL);
        else if (data.data_ == NULL || size_ != data.size_)
            result = false;
        else
            result = DataEqual<T>::equal(data_, data.data_, data.size_);
        return result;
    }

    /** Inequality operator.
     \param  data  Data to compare to
     \return       Whether inequal
    */
    bool operator!=(const ThisType& data) const {
        bool result;
        if (this == &data)
            result = false;
        else if (data_ == NULL)
            result = (data.data_ != NULL);
        else if (data.data_ == NULL || size_ != data.size_)
            result = true;
        else
            result = !DataEqual<T>::equal(data_, data.data_, data.size_);
        return result;
    }

    /** Make data unique -- no-op.
     - BitArray doesn't support sharing so this is a no-op
     .
     \return  This
    */
    ThisType& unshare() {
        return *this;
    }

    /** Resize while preserving existing data (modifier).
     \param  bitsize  New bit size to use
     \return          This
    */
    ThisType& resize(Size bitsize) {
        Size size = Bits<T,Size>::array_size(bitsize);
        if (size == 0) {
            // Null/Empty
            if (size_ > 0) {
                ::free(data_);
                data_ = EVO_PEMPTY;
                size_ = 0;
                bitsize_ = 0;
            }
        } else if (size_ != size) {
            // New positive size
            assert( size > 0 );
            const size_t size_bytes = (size_t)size * sizeof(Value);
            if (size_ > 0) {
                // Preserve existing items
                Value* const old_data = data_;
                data_ = (Value*)::malloc(size_bytes);

                const size_t save_size_bytes = (size_ < size ? size_ : size) * sizeof(Value);
                memcpy(data_, old_data, save_size_bytes);
                if (size_bytes > save_size_bytes)
                    memset((char*)data_ + save_size_bytes, 0, size_bytes - save_size_bytes);

                size_ = size;
                ::free(old_data);
            } else {
                // New array
                data_ = (T*)::malloc(size_bytes);
                memset(data_, 0, size_bytes);
            }
            size_ = size;
            bitsize_ = bitsize;
        } else {
            // Same array size
            if (bitsize < bitsize_) {
                // Clear removed bits
                const T mask = (Bits<T,Size>::RBIT << (bitsize_ - bitsize)) - 1;
                data_[size_ - 1] &= ~mask;
            }
            bitsize_ = bitsize;
        }
        return *this;
    }

    /** Resize as power of 2 while preserving existing data (modifier).
     - This makes sure size is always a power of 2, or 0 if empty
     - If desired size isn't a power of 2, this increases it to the next power of 2
     .
     \param  bitsize  New size to use
     \return          This
    */
    ThisType& resize_pow2(Size bitsize) {
        if (bitsize > 0)
            bitsize = size_pow2(bitsize);
        resize(bitsize);
        return *this;
    }

    // INTERNAL

    // Iterator support methods
    /** \cond impl */
    void iterInitMutable() { }
    const IterItem* iterFirst(IterKey& key) const {
        const IterItem* result;
        if (bitsize_ > 0) {
            key.offset = Bits<T,Size>::array_iter(key.state, data_, bitsize_);
            if (key.offset == NONE)
                result = NULL;
            else
                result = (const IterItem*)&key.offset;
        } else {
            key.offset = END;
            result = NULL;
        }
        return result;
    }
    const IterItem* iterNext(IterKey& key) const {
        const IterItem* result = NULL;
        if (key.offset != END) {
            key.offset = Bits<T,Size>::array_iternext(key.state);
            if (key.offset == NONE)
                result = NULL;
            else
                result = (const IterItem*)&key.offset;
        }
        return result;
    }
    /** \endcond */

private:
    T*   data_;
    Size size_;
    Size bitsize_;

    template<class> friend class BitArraySubsetT;
};

///////////////////////////////////////////////////////////////////////////////

/** Equality operator.
 \param  a  First subset to compare
 \param  b  Second subset to compare
 \return    Whether equal
*/
template<class T>
inline bool operator==(const BitArraySubsetT< BitArrayT<T> >& a, const BitArrayT<T>& b) {
    return (a == BitArraySubsetT< BitArrayT<T> >(b));
}

/** Inequality operator.
 \param  a  First subset to compare
 \param  b  Second subset to compare
 \return    Whether inequal
*/
template<class T>
inline bool operator!=(const BitArraySubsetT< BitArrayT<T> >& a, const BitArrayT<T>& b) {
    return !(a == BitArraySubsetT< BitArrayT<T> >(b));
}

///////////////////////////////////////////////////////////////////////////////

typedef BitArrayT<> BitArray;                       ///< Default dynamic bit array container -- see BitArrayT
typedef BitArraySubsetT<BitArray> BitArraySubset;   ///< Default subset of a BitArray -- see BitArraySubsetT

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
