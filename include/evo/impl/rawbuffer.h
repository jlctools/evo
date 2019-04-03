// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file rawbuffer.h Evo RawBuffer classes. */
#pragma once
#ifndef INCL_evo_impl_rawbuffer_h
#define INCL_evo_impl_rawbuffer_h

#include "sys.h"
#include "../type.h"

// Disable certain MSVC warnings for this file
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4458)
#endif

namespace evo {
/** \addtogroup EvoCore */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Simple raw memory buffer.
 - Members are public for quick access and simple low-level interface
   - Access buffer with 'data' member, size used with 'used'
   .
 - Use resize() to resize buffer
 - Use insert() to make room for new items
 - Use remove() to remove items
*/
struct RawBuffer {
    char* data;         ///< Buffer data pointer, NULL if empty (size=0)
    ulong size;         ///< Buffer size (capacity) in bytes -- do not modify, use: resize(), minsize(), reset(), ref()
    ulong used;         ///< Buffer size in use in bytes
    bool  owned;        ///< Whether this owns the buffer and must free it

    /** Constructor. */
    RawBuffer() : data(NULL), size(0), used(0), owned(false)
        { }

    /** Constructor. */
    RawBuffer(ulong size) : data(NULL), size(0), used(0), owned(false)
        { resize(size); }

    /** Destructor. */
    ~RawBuffer() {
        if (owned)
            ::free(data);
    }

    /** Get size available.
     \return  Size available at end (size - used)
    */
    ulong avail() const {
        assert( used <= size );
        return size - used;
    }

    /** Get size up to size available.
     \param  checksize  Size to check against size available
     \return            Size available at end (size - used) or `checksize`, whichever is smaller
    */
    ulong avail(ulong checksize) const {
        assert(used <= size);
        const ulong size_avail = size - used;
        return (checksize > size_avail ? size_avail : checksize);
    }

    /** Reset to empty state without a buffer.
     - This is useful for clearing a reference to another buffer
     .
     \return  This
    */
    RawBuffer& reset() {
        if (owned)
            ::free(data);
        data  = NULL;
        size  = 0;
        used  = 0;
        owned = false;
        return *this;
    }

    /** Reference another buffer.
     - Use to access and/or modify another buffer
     - This will not take ownership of the buffer (will not free it)
     .
     \param  data  Buffer data pointer, must not be NULL
     \param  size  Buffer size (capacity) in bytes, must be > 0
     \param  used  Buffer size in use in bytes, must be <= size
     \return       This
    */
    RawBuffer& ref(char* data, ulong size, ulong used=0) {
        assert( data != NULL );
        if (owned)
            ::free(this->data);
        this->data = data;
        this->size = size;
        this->used = used;
        owned      = false;
        return *this;
    }

    /** Resize buffer.
     - This updates 'used' if the new size truncates items
     - If newsize is the same as current size this does nothing
     - If newsize is 0 this will free the buffer (if owned) and reset the data
     - If newsize is positive this will: allocate a new buffer, copy existing data if needed (up to 'used'), and free the old buffer (if owned)
     .
     \param  newsize  New size in bytes, 0 to free
     \return          This
    */
    RawBuffer& resize(ulong newsize) {
        if (newsize == 0) {
            // Clear
            if (size > 0) {
                if (owned)
                    ::free(data);
                data  = NULL;
                size  = 0;
                used  = 0;
                owned = false;
            }
        } else if (size != newsize) {
            // New positive size
            assert( newsize > 0 );
            if (used > 0) {
                // Preserve existing items
                if (used > newsize)
                    used = newsize;
                char* const old_data = data;
                data = (char*)::malloc((size_t)newsize);
                memcpy(data, old_data, (size_t)used);
                if (owned)
                    ::free(old_data);
            } else {
                // New array
                if (owned)
                    ::free(data);
                data = (char*)::malloc((size_t)newsize);
            }
            size  = newsize;
            owned = true;
        }
        return *this;
    }

    /** Resize buffer if smaller than minimum size.
     - This calls resize(), if needed
     .
     \param  min  Minimum size in bytes
     \return      This
    */
    RawBuffer& minsize(ulong min) {
        if (min > size)
            resize(min);
        return *this;
    }

    /** Clear buffer.
     - This sets 'used' to 0
     .
     \return      This
    */
    RawBuffer& clear() {
        used   = 0;
        return *this;
    }

    /** Make room to insert items.
     - This shifts items over to make room and updates 'used' but doesn't actually insert
     .
     \param  index  Insert index, END to append
     \param  size   Insert size in bytes, must be postive
     \return        Actual insert index
    */
    ulong insert(ulong index, ulong size=1) {
        ulong tempsize = used + size;
        if (tempsize > this->size)
            resize(tempsize);
        if (index < used) {
            // Insert
            memmove(data+index+size, data+index, used-index);
        } else {
            // Append
            index = used;
        }
        used += size;
        return index;
    }

    /** Remove items.
     - This removes items and updates 'used'
     .
     \param  index  Insert index
     \param  size   Remove size in bytes
     \return        This
    */
    RawBuffer& remove(ulong index, ulong size=1) {
        if (index < used && size > 0) {
            // Size after removed items
            size_t tempsize = used - index;
            if (size >= tempsize)
                tempsize = 0;
            else
                tempsize -= size;

            // Remove
            if (tempsize > 0) {
                // Shift items after removed
                memmove(data+index, data+index+size, tempsize);
                used -= size;
            } else {
                // Remove all items after index
                used = index;
            }
        }
        return *this;
    }

    /** Flush data already read from buffer using offset.
     - This moves data at offset to beginning of buffer to make room for more data
     .
     \param  offset  Offset to use and update, will be set to 0 [in/out]
    */
    void flush(ulong& offset) {
        assert( used <= size );
        if (offset >= used) {
            offset = 0;
            used   = 0;
        } else if (offset > 0) {
            used -= offset;
            memmove(data, data+offset, used);
            offset = 0;
        }
    }

    /** Read from buffer using offset.
     - This copies from offset (data+offset) and increments offset
     .
     \param  offset    Offset to use and update [in/out]
     \param  buf       Buffer to read/copy to
     \param  readsize  Read size in bytes, reduced if too large
     \return           Actual size read/copied, may be less than readsize if end reached (offset=used)
    */
    ulong read(ulong& offset, char* buf, ulong readsize) {
        if (offset < used) {
            ulong maxsize = used - offset;
            if (readsize > maxsize)
                readsize = maxsize;
            memcpy(buf, data+offset, readsize);
            offset += readsize;
        } else
            readsize = 0;
        return readsize;
    }

    /** Write at end of buffer.
     - This appends to buffer and increments used
     - If write is too big for buffer it will be truncated to fit
     .
     \param  buf        Buffer to write from
     \param  writesize  Write size in bytes, reduced if too large
     \return            Actual size written, may be less than writesize if buffer is full
    */
    ulong write(const char* buf, ulong writesize) {
        if (used < size) {
            ulong maxsize = size - used;
            if (writesize > maxsize)
                writesize = maxsize;
            memcpy(data+used, buf, writesize);
            used += writesize;
        } else
            writesize = 0;
        return writesize;
    }

    /** Write character at end of buffer.
     - This appends to buffer and increments used
     - If count is too big for buffer it will be truncated to fit
     .
     \param  ch     Character to write
     \param  count  Character repeat count, reduced if too large
     \return        Actual size written, may be less than count if buffer is full
    */
    ulong write(char ch, ulong count=1) {
        if (used < size) {
            const ulong maxsize = size - used;
            if (count > maxsize)
                count = maxsize;
            memset(data+used, ch, count);
            used += count;
        } else
            count = 0;
        return count;
    }

private:
    // Disable copying
    RawBuffer(const RawBuffer&);
    RawBuffer& operator=(const RawBuffer&);
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif
#endif
