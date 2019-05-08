// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file strtok.h Evo string tokenizers. */
#pragma once
#ifndef INCL_evo_strtok_h
#define INCL_evo_strtok_h

#include "substring.h"

namespace evo {
/** \addtogroup EvoTokenizers
Evo string tokenizers
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** Base tokenizer class -- see StrTok and StrTokR. */
class StrTokBase {
public:
    typedef StrTokBase      BaseType;    ///< Base type
    typedef SubString::Size Size;        ///< %String size type

    /** Get current index before next token.
     \return  Current index, END if at end
    */
    Size index() const
        { return index_; }

    /** Get current delimiter before next token.
     \return  Current delimiter, null if none or at end
    */
    Char delim() const
        { return delim_; }

    /** Get current token value from last call to next().
     \return  Current token value
    */
    const SubString& value() const
        { return value_; }

protected:
    SubString string_;        ///< %String being tokenized, NULL for none
    Size      index_;        ///< Current index, END when at end
    Char      delim_;        ///< Current delimiter, null when none or at end
    SubString value_;        ///< Current value

    /** Default constructor creates empty tokenizer. */
    StrTokBase()
        { index_ = END; }

    /** Default constructor creates empty tokenizer. */
    StrTokBase(const StringBase& string) :
        string_( string ),
        index_(  END )
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTokBase(const BaseType& src) :
        string_ ( src.string_ ),
        index_(   src.index_ ),
        delim_(   src.delim_ ),
        value_(   src.value_ )
        { }

    /** Copy data. */
    void copy(const BaseType& src) {
        this->string_ = src.string_;
        this->index_  = src.index_;
        this->delim_  = src.delim_;
        this->value_  = src.value_;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %String forward tokenizer.
 - Variants: \link StrTokWord\endlink
 - This skips whitespace (spaces, tabs) between delimiters so tokens will not start or end with any whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For reverse tokenizing see: StrTokR
 - For "strict" tokenizing (without skipping whitespace) see: StrTokS, StrTokRS
 - Note: All tokenizers (including forward and reverse) implement the same basic interface (excluding `next*()` variants)
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one, two, three";

    // Tokens:
    //  one
    //  two
    //  three
    StrTok tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
*/
class StrTok : public StrTokBase {
public:
    typedef StrTok          ThisType;    ///< This type
    typedef StrTokBase      BaseType;    ///< Base type
    typedef SubString::Size Size;        ///< %String size type

    /** Default constructor creates empty tokenizer. */
    StrTok()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTok(const ThisType& src) : StrTokBase((const BaseType&)src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTok(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string.
     - Call next() or nextw() for each token
     .
     \param  str  %String to tokenize
    */
    StrTok(const StringBase& str) : StrTokBase(str)
        { impl_reset(); }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from beginning. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    ThisType& operator=(const StringBase& str) {
        this->string_ = str;
        this->value_.set();
        this->delim_.set();
        impl_reset();
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    ThisType& reset() {
        this->delim_.set();
        this->value_.set();
        impl_reset();
        return *this;
    }

    /** Find next token using delimiter. Call value() to get token value.
     - This will skip leading whitespace (spaces, tabs) before and after next token
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool next(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind < size && ((ch=data[ind]) == ' ' || ch == '\t'); )
            ++ind;
        if (ind == size) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        data += ind;
        size -= ind;
        const char* p = (char*)::memchr(data, delim, size);
        if (p == NULL) {
            this->value_.set(data, size).stripr();
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(p - data);
            this->value_.set(data, size).stripr();
            this->delim_ = delim;
            ind = ind + size + 1;
        }
        return true;
    }

    /** Find next token using delimiter with quoting support. Call value() to get token value.
     - This will skip leading whitespace (spaces, tabs) before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     - See \ref SmartQuoting
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextq(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind < size && ((ch=data[ind]) == ' ' || ch == '\t'); )
            ++ind;
        if (ind == size) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        const char* start = data + ind;
        const char* end = data + size;
        const char* startq;
        const char* endq;
        const char* p = str_scan_endq(startq, endq, start, end, delim);
        size = (Size)(endq - startq);
        if (p == end) {
            this->value_.set(startq, size);
            this->delim_.set();
            ind = END;
        } else {
            this->value_.set(startq, size);
            this->delim_ = *p;
            ind = (Size)(p + 1 - data);
        }
        if (startq == start)
            this->value_.strip();
        return true;
    }

    /** Find next token using word delimiter. Call value() to get token value.
     - Same as next(char) except this will also skip any leading extra delimiters before next token
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextw(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace and dup delims
        const char* data = this->string_.data_;
        for (char ch; ind < size && ((ch=data[ind]) == ' ' || ch == '\t' || ch == delim); )
            ++ind;
        if (ind == size) {
            this->value_.set();
            this->delim_.set();
            ind = END;
            return false;
        }

        // Extract token
        data += ind;
        size -= ind;
        const char* p = (char*)::memchr(data, delim, size);
        if (p == NULL) {
            this->value_.set(data, size).stripr();
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(p - data);
            this->value_.set(data, size).stripr();
            this->delim_ = delim;
            ind = ind + size + 1;
        }
        return true;
    }

    /** Find next token using any of given delimiters. Call value() to get token value.
     - This will skip leading whitespace (spaces, tabs) before and after next token
     .
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const StringBase& delims) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind < size && ((ch=data[ind]) == ' ' || ch == '\t'); )
            ++ind;
        if (ind == size) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        size -= ind;
        const char* start = data + ind;
        const char* end = start + size;
        const char* p = str_scan_delim(start, end, delims.data_, delims.size_);
        size = (Size)(p - start);
        this->value_.set(start, size).strip();
        if (p == end) {
            this->delim_.set();
            ind = END;
        } else {
            this->delim_ = *p;
            ind = (Size)(p + 1 - data);
        }
        return true;
    }

    /** Find next token using any of given delimiters with quoting support. Call value() to get token value.
     - This will skip leading whitespace (spaces, tabs) before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     - For best performance set `ws_delim` to 0 or the whitespace delimiter to skip whitespace delimiter detection
     .
     \param  delims    Delimiters to use -- must not have more than 1 whitespace character (space, tab, newline)
     \param  ws_delim  Use to specify a whitespace char in `delims` so it's handled correctly, 1 to auto-detect, 0 if no whitespace delim
     \return           Whether next token was found, false if no more
    */
    bool nextanyq(const StringBase& delims, char ws_delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind < size && ((ch=data[ind]) == ' ' || ch == '\t'); )
            ++ind;
        if (ind == size) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Detect whitespace delim
        char ws_delim_char;
        if (ws_delim == 1) {
            const StrSizeT ws_i = SubString(delims).findany(" \t\r\n", 4);
            ws_delim_char = (ws_i == NONE ? 0 : delims.data_[ws_i]);
        } else
            ws_delim_char = ws_delim;

        // Extract token
        const char* start = data + ind;
        const char* end = data + size;
        const char* startq;
        const char* endq;
        const char* p = str_scan_endq(startq, endq, start, end, delims.data_, delims.size_, ws_delim_char);
        size = (Size)(endq - startq);
        if (p == end) {
            this->value_.set(startq, size);
            this->delim_.set();
            ind = END;
        } else {
            this->value_.set(startq, size);
            this->delim_ = *p;
            ind = (Size)(p + 1 - data);
        }
        if (startq == start)
            this->value_.strip();
        return true;
    }

    /** Find next token using any of given delimiters with quoting support. Call value() to get token value.
     - This will skip leading whitespace (spaces, tabs) before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     - For best performance use `nextanyq(const StringBase&,char)` and set `ws_delim` to 0 or the whitespace delimiter to skip whitespace delimiter detection
     .
     \param  delims  Delimiters to use -- must not have more than 1 whitespace character (space, tab, newline)
     \return         Whether next token was found, false if no more
    */
    bool nextanyq(const StringBase& delims) {
        return nextanyq(delims, 1); // 1 to detect a whitespace delim
    }

    /** Advance current position for next token by skipping whitespace.
     - This gives the index where the next token starts
     - This is useful for advanced parsing that needs to look at the context of next token (indent amount, characters before token, etc)
     .
     \return  Current index for next token
    */
    Size skipws() {
        const char* data = this->string_.data_;
        const Size  size = this->string_.size_;
        Size&       ind  = this->index_;
        for (char ch; ind < size && ((ch=data[ind]) == ' ' || ch == '\t'); )
            ++ind;
        return ind;
    }

    /** Split delimited string into item list using next().
     - This tokenizes and adds each item to list, using convert() for conversion to list item type
     - String must be convertible to list item type via convert()
     - See String::join() to join list back into string
     .
     \tparam  C  List container for items -- inferred from items parameter
     \tparam  T  %String type to split -- inferred from str parameter
     \param  items  List to add items to [in/out]
     \param  str    %String to tokenize
     \param  delim  Delimiter to use
     */
    template<class C,class T>
    static typename C::Size split(C& items, const T& str, char delim=',') {
        typename C::Size count = 0;
        ThisType tok(str);
        for (; tok.next(delim); ++count)
            items.add(tok.value().convert<typename C::Item>());
        return count;
    }

    /** Split delimited string to extract token at index.
     - This will tokenize until token at index is found
     .
     @tparam  T  %String type to tokenize -- inferred from str parameter
     @param  str    %String to tokenize
     @param  index  Token index to extract
     @param  delim  Delimiter to use
     @return        Result token, set to null if not found
     */
    template<class T>
    static SubString splitat(const T& str, Size index, char delim=',') {
        SubString result;
        ThisType tok(str);
        for (Size i=0; tok.next(delim); ++i)
            if (i == index)
                { result = tok.value(); break; }
        return result;
    }

private:
    void impl_reset() {
        const char* str_data_ = this->string_.data_;
        Size        str_size_ = this->string_.size_;
        Size&       ind_ =      this->index_;
        char ch;
        ind_ = 0;
        while ( ind_ < str_size_ && ((ch=str_data_[ind_]) == ' ' || ch == '\t') )
            ++ind_;
        if (ind_ >= str_size_)
            ind_ = END;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %String reverse tokenizer.
 - Variants: \link StrTokWordR\endlink
 - This skips whitespace between delimiters so tokens will not start or end with any whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For forward tokenizing see: StrTok
 - For "strict" tokenizing (without skipping whitespace) see: StrTokRS, StrTokS
 - Note: All tokenizers (including forward and reverse) implement the same basic interface (excluding `next*()` variants)
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one, two, three";

    // Tokens:
    //  three
    //  two
    //  one
    StrTokR tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
*/
class StrTokR : public StrTokBase {
public:
    typedef StrTokR         ThisType;        ///< This type
    typedef StrTokBase      BaseType;        ///< Base type
    typedef SubString::Size Size;            ///< %String size type

    /** Default constructor creates empty tokenizer. */
    StrTokR() : StrTokBase()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTokR(const ThisType& src) : StrTokBase(src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTokR(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string. Call next() or nextw() for each token.
     \param  str  %String to tokenize
    */
    StrTokR(const StringBase& str) : StrTokBase(str)
        { impl_reset(); }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from end. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    ThisType& operator=(const StringBase& str) {
        this->string_ = str;
        this->value_.set();
        this->delim_.set();
        impl_reset();
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    ThisType& reset() {
        this->value_.set();
        this->delim_.set();
        impl_reset();
        return *this;
    }

    /** Find next token using delimiter (in reverse order). Call value() to get token value.
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool next(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind > 0 && ((ch=data[ind - 1]) == ' ' || ch == '\t'); )
            --ind;
        if (ind == 0) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        const char* p = string_memrchr(data, delim, ind);
        if (p == NULL) {
            this->value_.set(data, ind).stripl();
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(data + ind - (++p));
            this->value_.set(p, size).stripl();
            this->delim_ = delim;
            ind = ind - size - 1;
        }
        return true;
    }

    /** Find next token with quoting support using delimiter (in reverse order) with quoting support. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextq(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind > 0 && ((ch=data[ind - 1]) == ' ' || ch == '\t'); )
            --ind;
        if (ind == 0) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        const char* end = data + ind;
        const char* startq;
        const char* endq;
        const char* p = str_scan_endq_r(startq, endq, data, end, delim);
        size = (Size)(endq - startq);
        p = str_scan_nws_r(data, p, delim);
        if (p == data) {
            this->delim_.set();
            ind = END;
        } else {
            --p;
            this->delim_ = *p;
            ind = (Size)(p - data);
        }
        this->value_.set(startq, size);
        if (endq == end)
            this->value_.strip();
        return true;
    }

    /** Find next token using word delimiter (in reverse order). Call value() to get token value.
     - Same as next(char) except duplicate delimiters are skipped
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextw(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind > 0 && ((ch=data[ind - 1]) == ' ' || ch == '\t' || ch == delim); )
            --ind;
        if (ind == 0) {
            this->value_.set();
            this->delim_.set();
            ind = END;
            return false;
        }

        // Extract token
        const char* p = string_memrchr(data, delim, ind);
        if (p == NULL) {
            this->value_.set(data, ind).stripl();
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(data + ind - (++p));
            this->value_.set(p, size).stripl();
            this->delim_ = delim;
            ind = ind - size - 1;
        }
        return true;
    }

    /** Find next token using any of given delimiters (in reverse order). Call value() to get token value.
     - This will skip leading whitespace before and after next token
     .
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const StringBase& delims) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind > 0 && ((ch=data[ind - 1]) == ' ' || ch == '\t'); )
            --ind;
        if (ind == 0) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        const char* end = data + ind;
        const char* p = str_scan_delim_r(data, end, delims.data_, delims.size_);
        size = (Size)(end - p);
        this->value_.set(p, size).stripl();
        if (p == data) {
            this->delim_.set();
            ind = END;
        } else {
            --p;
            this->delim_ = *p;
            ind = (Size)(p - data);
        }
        return true;
    }

    /** Find next token using any of given delimiters (in reverse order) with quoting support. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     - For best performance set `ws_delim` to 0 or the whitespace delimiter to skip whitespace delimiter detection
     .
     \param  delims    Delimiters to use -- must not have more than 1 whitespace character (space, tab, newline)
     \param  ws_delim  Use to specify a whitespace char in `delims` so it's handled correctly, 1 to auto-detect, 0 if no whitespace delim
     \return           Whether next token was found, false if no more
    */
    bool nextanyq(const StringBase& delims, char ws_delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip whitespace
        const char* data = this->string_.data_;
        for (char ch; ind > 0 && ((ch=data[ind - 1]) == ' ' || ch == '\t'); )
            --ind;
        if (ind == 0) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Detect whitespace delim
        char ws_delim_char;
        if (ws_delim == 1) {
            const StrSizeT ws_i = SubString(delims).findany(" \t\r\n", 4);
            ws_delim_char = (ws_i == NONE ? 0 : delims.data_[ws_i]);
        } else
            ws_delim_char = ws_delim;

        // Extract token
        const char* end = data + ind;
        const char* startq;
        const char* endq;
        const char* p = str_scan_endq_r(startq, endq, data, end, delims.data_, delims.size_, ws_delim_char);
        size = (Size)(endq - startq);
        p = str_scan_nws_r(data, p, ws_delim_char);
        if (p == data) {
            this->delim_.set();
            ind = END;
        } else {
            --p;
            this->delim_ = *p;
            ind = (Size)(p - data);
        }
        this->value_.set(startq, size);
        if (endq == end)
            this->value_.strip();
        return true;
    }

    /** Find next token using any of given delimiters (in reverse order) with quoting support. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     - For best performance set `ws_delim` to 0 or the whitespace delimiter to skip whitespace delimiter detection
     .
     \param  delims  Delimiters to use -- must not have more than 1 whitespace character (space, tab, newline)
     \return         Whether next token was found, false if no more
    */
    bool nextanyq(const StringBase& delims) {
        return nextanyq(delims, 1); // 1 to detect a whitespace delim
    }

    /** Split delimited string into item list using next() (in reverse order).
     - This tokenizes and adds each item to list, using convert() for conversion to list item type
     - String must be convertible to list item type via convert()
     - See String::join() to join list back into string
     .
     \tparam  C  List container for items -- inferred from items parameter
     \tparam  T  %String type to split -- inferred from str parameter
     \param  items  List to add items to [in/out]
     \param  str    %String to tokenize
     \param  delim  Delimiter to use
     */
    template<class C,class T>
    static typename C::Size split(C& items, const T& str, char delim=',') {
        typename C::Size count = 0;
        ThisType tok(str);
        for (; tok.next(delim); ++count)
            items.add(tok.value().convert<typename C::Item>());
        return count;
    }

    /** Split delimited string to extract token at index (in reverse order).
     - This will tokenize until token at index is found
     .
     @tparam  T  %String type to tokenize -- inferred from str parameter
     @param  str    %String to tokenize
     @param  index  Token index to extract
     @param  delim  Delimiter to use
     @return        Result token, set to null if not found
     */
    template<class T>
    static SubString splitat(const T& str, Size index, char delim=',') {
        SubString result;
        ThisType tok(str);
        for (Size i=0; tok.next(delim); ++i)
            if (i == index)
                { result = tok.value(); break; }
        return result;
    }

private:
    void impl_reset() {
        const char* str_data_ = this->string_.data_;
        Size&       ind_      = this->index_;
        char ch;
        ind_ = this->string_.size_;
        while ( ind_ > 0 && ((ch=str_data_[ind_-1]) == ' ' || ch == '\t') )
            --ind_;
        if (ind_ == 0)
            ind_ = END;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %String forward tokenizer (strict).
 - Variants: \link StrTokWordS\endlink
 - This does not skip whitespace so tokens may start or end with whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For reverse "strict" tokenizing see: StrTokRS
 - For "non-strict" tokenizing (skipping whitespace) see: StrTok, StrTokR
 - Note: All tokenizers (including forward and reverse) implement the same basic interface (excluding `next*()` variants)
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one,two,three";

    // Tokens:
    //  one
    //  two
    //  three
    StrTokS tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
*/
class StrTokS : public StrTokBase {
public:
    typedef StrTokS         ThisType;        ///< This type
    typedef StrTokBase      BaseType;        ///< Base type
    typedef SubString::Size Size;            ///< %String size type

    /** Default constructor creates empty tokenizer. */
    StrTokS()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTokS(const ThisType& src) : StrTokBase(src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTokS(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string. Call next() or nextw() for each token.
     \param  str  %String to tokenize
    */
    StrTokS(const StringBase& str) : StrTokBase(str)
        { this->index_ = (this->string_.size_ > 0 ? 0 : (Size)END); }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from beginning. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    ThisType& operator=(const StringBase& str) {
        this->string_ = str;
        this->value_.set();
        this->delim_.set();
        this->index_ = (this->string_.size_ > 0 ? 0 : (Size)END);
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    ThisType& reset() {
        this->value_.set();
        this->delim_.set();
        this->index_ = (this->string_.size_ > 0 ? 0 : (Size)END);
        return *this;
    }

    /** Find next token using delimiter. Call value() to get token value.
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool next(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        } else if (ind == size) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        size -= ind;
        const char* data = this->string_.data_ + ind;
        const char* p = (char*)::memchr(data, delim, size);
        if (p == NULL) {
            this->value_.set(data, size);
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(p - data);
            this->value_.set(data, size);
            this->delim_ = delim;
            ind = ind + size + 1;
        }
        return true;
    }

    /** Find next token using word delimiter. Call value() to get token value.
     - Same as next(char) except duplicate delimiters are skipped
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextw(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip dup delims
        const char* data = this->string_.data_;
        while (ind < size && data[ind] == delim)
            ++ind;
        if (ind == size) {
            this->value_.set();
            this->delim_.set();
            ind = END;
            return false;
        }

        // Extract token
        data += ind;
        size -= ind;
        const char* p = (char*)::memchr(data, delim, size);
        if (p == NULL) {
            this->value_.set(data, size);
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(p - data);
            this->value_.set(data, size);
            this->delim_ = delim;
            ind = ind + size + 1;
        }
        return true;
    }

    /** Find next token using any of given delimiters. Call value() to get token value.
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const StringBase& delims) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        } else if (ind == size) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        size -= ind;
        const char* data = this->string_.data_;
        const char* start = data + ind;
        const char* end = start + size;
        const char* p = str_scan_delim(start, end, delims.data_, delims.size_);
        size = (Size)(p - start);
        this->value_.set(start, size);
        if (p == end) {
            this->delim_.set();
            ind = END;
        } else {
            this->delim_ = *p;
            ind = (Size)(p + 1 - data);
        }
        return true;
    }

    /** Split delimited string into item list using next().
     - This tokenizes and adds each item to list, using convert() for conversion to list item type
     - String must be convertible to list item type via convert()
     - See String::join() to join list back into string
     .
     \tparam  C  List container for items -- inferred from items parameter
     \tparam  T  %String type to split -- inferred from str parameter
     \param  items  List to add items to [in/out]
     \param  str    %String to tokenize
     \param  delim  Delimiter to use
     */
    template<class C,class T>
    static typename C::Size split(C& items, const T& str, char delim=',') {
        typename C::Size count = 0;
        ThisType tok(str);
        for (; tok.next(delim); ++count)
            items.add(tok.value().convert<typename C::Item>());
        return count;
    }

    /** Split delimited string to extract token at index.
     - This will tokenize until token at index is found
     .
     @tparam  T  %String type to tokenize -- inferred from str parameter
     @param  str    %String to tokenize
     @param  index  Token index to extract
     @param  delim  Delimiter to use
     @return        Result token, set to null if not found
     */
    template<class T>
    static SubString splitat(const T& str, Size index, char delim=',') {
        SubString result;
        ThisType tok(str);
        for (Size i=0; tok.next(delim); ++i)
            if (i == index)
                { result = tok.value(); break; }
        return result;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %String reverse tokenizer (strict).
 - Variants: \link StrTokWordRS\endlink
 - This does not skip whitespace so tokens may start or end with whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For forward "strict" tokenizing see: StrTokS
 - For "non-strict" tokenizing (skipping whitespace) see: StrTokR, StrTok
 - Note: All tokenizers (including forward and reverse) implement the same basic interface (excluding `next*()` variants)
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one,two,three";

    // Tokens:
    //  three
    //  two
    //  one
    StrTokRS tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
*/
class StrTokRS : public StrTokBase {
public:
    typedef StrTokRS ThisType;        ///< This type
    typedef StrTokBase      BaseType;        ///< Base type
    typedef SubString::Size Size;            ///< %String size type

    /** Default constructor creates empty tokenizer. */
    StrTokRS() : StrTokBase()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTokRS(const ThisType& src) : StrTokBase(src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    StrTokRS(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string. Call next() or nextw() for each token.
     \param  str  %String to tokenize
    */
    StrTokRS(const StringBase& str) : StrTokBase(str)
        { this->index_ = (str.size_ > 0 ? str.size_ : END); }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from end. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    ThisType& operator=(const StringBase& str) {
        this->string_ = str;
        this->index_  = (str.size_ > 0 ? str.size_ : END);
        this->value_.set();
        this->delim_.set();
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    ThisType& reset() {
        this->index_ = (this->string_.size_ > 0 ? this->string_.size_ : END);
        this->value_.set();
        this->delim_.set();
        return *this;
    }

    /** Find next token using delimiter (in reverse order). Call value() to get token value.
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool next(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        } else if (ind == 0) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        const char* data = this->string_.data_;
        const char* p = string_memrchr(data, delim, ind);
        if (p == NULL) {
            this->value_.set(data, ind);
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(data + ind - (++p));
            this->value_.set(p, size);
            this->delim_ = delim;
            ind = ind - size - 1;
        }
        return true;
    }

    /** Find next token using word delimiter (in reverse order). Call value() to get token value.
     - Same as next(char) except duplicate delimiters are skipped
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextw(char delim) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        }

        // Skip dup delims
        const char* data = this->string_.data_;
        while (ind > 0 && data[ind - 1] == delim)
            --ind;
        if (ind == 0) {
            this->value_.set();
            this->delim_.set();
            ind = END;
            return false;
        }

        // Extract token
        const char* p = string_memrchr(data, delim, ind);
        if (p == NULL) {
            this->value_.set(data, ind);
            this->delim_.set();
            ind = END;
        } else {
            size = (Size)(data + ind - (++p));
            this->value_.set(p, size);
            this->delim_ = delim;
            ind = ind - size - 1;
        }
        return true;
    }

    /** Find next token using any of given delimiters (in reverse order). Call value() to get token value.
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const StringBase& delims) {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        } else if (ind == 0) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        const char* data = this->string_.data_;
        const char* end = data + ind;
        const char* p = str_scan_delim_r(data, end, delims.data_, delims.size_);
        size = (Size)(end - p);
        this->value_.set(p, size);
        if (p == data) {
            this->delim_.set();
            ind = END;
        } else {
            --p;
            this->delim_ = *p;
            ind = (Size)(p - data);
        }
        return true;
    }

    /** Split delimited string into item list using next() (in reverse order).
     - This tokenizes and adds each item to list, using convert() for conversion to list item type
     - String must be convertible to list item type via convert()
     - See String::join() to join list back into string
     .
     \tparam  C  List container for items -- inferred from items parameter
     \tparam  T  %String type to split -- inferred from str parameter
     \param  items  List to add items to [in/out]
     \param  str    %String to tokenize
     \param  delim  Delimiter to use
     */
    template<class C,class T>
    static typename C::Size split(C& items, const T& str, char delim=',') {
        typename C::Size count = 0;
        ThisType tok(str);
        for (; tok.next(delim); ++count)
            items.add(tok.value().convert<typename C::Item>());
        return count;
    }

    /** Split delimited string to extract token at index (in reverse order).
     - This will tokenize until token at index is found
     .
     @tparam  T  %String type to tokenize -- inferred from str parameter
     @param  str    %String to tokenize
     @param  index  Token index to extract
     @param  delim  Delimiter to use
     @return        Result token, set to null if not found
     */
    template<class T>
    static SubString splitat(const T& str, Size index, char delim=',') {
        SubString result;
        ThisType tok(str);
        for (Size i=0; tok.next(delim); ++i)
            if (i == index)
                { result = tok.value(); break; }
        return result;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %String tokenizer adapter used internally to create variants of existing tokenizers -- do not use directly.
 - This is used to create tokenizer variants with different next() behavior
 - See \link StrTokWord\endlink for example
 .
 \tparam  T        %String tokenizer to extend
 \tparam  NextCh   Pointer to member function for next() to call
 \tparam  NextAny  Pointer to member function for nextany() to call
*/
template<class T, bool (T::*NextCh)(char), bool (T::*NextAny)(const StringBase&)=&T::nextany>
struct StrTokVariant : public T {
    typedef StrTokVariant<T,NextCh,NextAny> ThisType;   ///< This type
    typedef StrTokBase                      BaseType;   ///< Root base type
    typedef SubString::Size                 Size;       ///< %String size type

    StrTokVariant()
        { }
    StrTokVariant(const ThisType& src) : T((const T&)src)
        { }
    StrTokVariant(const BaseType& src) : T(src)
        { }
    StrTokVariant(const StringBase& str) : T(str)
        { }

    ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }
    ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }
    ThisType& operator=(const StringBase& str)
        { T::operator=(str); return *this; }

    bool next(char delim)
        { return ( ((T*)this)->*NextCh )(delim); }
    bool nextany(const StringBase& delims)
        { return ( ((T*)this)->*NextAny )(delims); }

    template<class C,class S>
    static typename C::Size split(C& items, const S& str, char delim=',') {
        typename C::Size count = 0;
        T tok(str);
        for (; (tok.*NextCh)(delim); ++count)
            items.add( tok.value().template convert<typename C::Item>() );
        return count;
    }

    template<class S>
    static SubString splitat(const S& str, Size index, char delim=',') {
        SubString result;
        T tok(str);
        for (Size i=0; (tok.*NextCh)(delim); ++i)
            if (i == index)
                { result = tok.value(); break; }
        return result;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** %String forward tokenizer based on StrTok with quoted token support.
 - This is the same as StrTok, except next() is overridden to behave as StrTok::nextq()
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one, \"two\", three";

    // Tokens:
    //  one
    //  two
    //  three
    StrTokQ tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
 */
typedef StrTokVariant<StrTok,&StrTok::nextq,&StrTok::nextanyq> StrTokQ;

/** %String reverse tokenizer based on StrTokR with quoted token support.
 - This is the same as StrTokR, except next() is overridden to behave as StrTokR::nextq()
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one, \"two\", three";

    // Tokens:
    //  three
    //  two
    //  one
    StrTokQR tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
 */
typedef StrTokVariant<StrTokR,&StrTokR::nextq,&StrTokR::nextanyq> StrTokQR;

///////////////////////////////////////////////////////////////////////////////

/** %String forward word tokenizer based on StrTok.
 - This is the same as StrTok, except next() is overridden to behave as StrTok::nextw()
 - This uses StrTokVariant, which may prevent inlining optimization on next() -- for absolute best performance use StrTok and StrTok::nextw() directly
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one, two, three";

    // Tokens:
    //  one
    //  two
    //  three
    StrTokWord tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
 */
typedef StrTokVariant<StrTok,&StrTok::nextw> StrTokWord;

/** %String reverse word tokenizer based on StrTokR.
 - This is the same as StrTokR, except next() is overridden to behave as StrTokR::nextw()
 - This uses StrTokVariant, which may prevent inlining optimization on next() -- for absolute best performance use StrTokR and StrTokR::nextw() directly
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one, two, three";

    // Tokens:
    //  three
    //  two
    //  one
    StrTokWordR tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
 */
typedef StrTokVariant<StrTokR,&StrTokR::nextw> StrTokWordR;

/** %String forward word tokenizer based on StrTokS (strict).
 - This is the same as StrTokS, except next() is overridden to behave as StrTokS::nextw()
 - This uses StrTokVariant, which may prevent inlining optimization on next() -- for absolute best performance use StrTokS and StrTokS::nextw() directly
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one,two,three";

    // Tokens:
    //  one
    //  two
    //  three
    StrTokWordS tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
 */
typedef StrTokVariant<StrTokS,&StrTokS::nextw> StrTokWordS;

/** %String reverse word tokenizer based on StrTokRS (strict).
 - This is the same as StrTokRS, except next() is overridden to behave as StrTokRS::nextw()
 - This uses StrTokVariant, which may prevent inlining optimization on next() -- for absolute best performance use StrTokRS and StrTokRS::nextw() directly
 .
Example:
\code
#include <evo/strtok.h>
#include <evo/io.h>
using namespace evo;

int main() {
    Console& c = con();
    SubString str = "one,two,three";

    // Tokens:
    //  three
    //  two
    //  one
    StrTokWordRS tok(str);
    while (tok.next(','))
        c.out << tok.value();

    return 0;
}
\endcode
 */
typedef StrTokVariant<StrTokRS,&StrTokRS::nextw> StrTokWordRS;

///////////////////////////////////////////////////////////////////////////////

/** %String line tokenizer.
 - This references target string without allocating memory -- results are undefined if target string is modified while referenced
 - Tokens do not include the newline character(s), whitespace is left as-is
 .

\par Example

\code
#include <evo/strtok.h>
#include <evo/string.h>
#include <evo/io.h>
using namespace evo;
Console& c = con();

int main() {
    String str = "one\ntwo\r\nthree";

    StrTokLine tok(str);
    while (tok.next())
        c.out << tok.value() << NL;

    return 0;
}
\endcode

Output:
\code{.unparsed}
one
two
three
\endcode
*/
class StrTokLine : public StrTokBase {
public:
    typedef StrTokLine      ThisType;    ///< This type
    typedef StrTokBase      BaseType;    ///< Base type
    typedef SubString::Size Size;        ///< %String size type

    /** \copydoc StrTok::StrTok() */
    StrTokLine()
        { }

    /** \copydoc StrTok::StrTok(const ThisType&) */
    StrTokLine(const ThisType& src) : StrTokBase((const BaseType&)src)
        { }

    /** \copydoc StrTok::StrTok(const BaseType&) */
    StrTokLine(const BaseType& src) : StrTokBase(src)
        { }

    /** \copydoc StrTok::StrTok(const StringBase&) */
    StrTokLine(const StringBase& str) : StrTokBase(str)
        { impl_reset(); }

    /** \copydoc StrTok::operator=(const ThisType&) */
    ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** \copydoc StrTok::operator=(const BaseType&) */
    ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** \copydoc StrTok::operator=(const StringBase&) */
    ThisType& operator=(const StringBase& str) {
        this->string_ = str;
        this->value_.set();
        this->delim_.set();
        impl_reset();
        return *this;
    }

    /** \copydoc StrTok::reset() */
    ThisType& reset() {
        this->delim_.set();
        this->value_.set();
        impl_reset();
        return *this;
    }

    /** Find next token by finding next newline or newline pair.
     - Call value() to get token value
     - This recognizes all the main newlines types, see \ref Newline
     - Note that this does NOT populate delim() -- it will always be null
     .
     \return  Whether next token was found, false if no more
    */
    bool next() {
        Size& ind  = this->index_;
        Size  size = this->string_.size_;
        if (ind > size) {
            this->value_.set();
            this->delim_.set();
            return false;
        } else if (ind == size) {
            this->value_.setempty();
            this->delim_.set();
            ind = END;
            return true;
        }

        // Extract token
        size -= ind;
        const char* data = this->string_.data_;
        const char* start = data + ind;
        const char* end = start + size;
        const char* p = str_scan_delim(start, end, '\r', '\n');
        size = (Size)(p - start);
        this->value_.set(start, size);
        this->delim_.set();
        if (p == end) {
            ind = END;
        } else {
            ind = (Size)(p + 1 - data);
            if (p + 1 < end && ((*p == '\r' && p[1] == '\n') || (*p == '\n' && p[1] == '\r')))
                ++ind;
        }
        return true;
    }

    /** Split string lines into list using next().
     - This tokenizes and adds each line to list, using convert() for conversion to list item type
     - String must be convertible to list item type via convert()
     - See String::join() to join list back into string
     .
     \tparam  C  List container for items -- inferred from `items` argument
     \tparam  T  %String type to split -- inferred from `str` argument
     \param  items  List to add items to [in/out]
     \param  str    %String to tokenize
    */
    template<class C,class T>
    static typename C::Size split(C& items, const T& str) {
        typename C::Size count = 0;
        ThisType tok(str);
        for (; tok.next(); ++count)
            items.add(tok.value().convert<typename C::Item>());
        return count;
    }

    /** Split string lines to extract token at line index.
     - This will tokenize until token at line index is found
     .
     @tparam  T  %String type to tokenize -- inferred from str parameter
     @param  str    %String to tokenize
     @param  index  Token line index to extract
     @return        Result token, set to null if not found
    */
    template<class T>
    static SubString splitat(const T& str, Size index) {
        SubString result;
        ThisType tok(str);
        for (Size i=0; tok.next(); ++i)
            if (i == index) {
                result = tok.value();
                break;
            }
        return result;
    }

private:
    void impl_reset() {
        Size  str_size_ = this->string_.size_;
        Size& ind_      = this->index_;
        ind_ = 0;
        if (ind_ >= str_size_)
            ind_ = END;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Helper for tokenizing using a break-loop.
 - A break-loop is a loop that always breaks at the end so doesn't actually loop, but also allows an early break to skip remaining code in the loop
 - This calls `TOK.next()` and if it fails this does a `break` to stop current loop
 - This makes tokenizing and validating fields more readable by condensing repetitive if-statements
 - See also: EVO_TOK_OR_BREAK()
 .

\par Example

These examples use `assert()` to represent field validation.

\code{.cpp}
#include <evo/strtok.h>
using namespace evo;

int main() {
    StrTok tok("1,2,3");
    for (;;) {
        // Field 1
        EVO_TOK_NEXT_OR_BREAK(tok, ',');
        assert(tok.value() == "1");

        // Field 2
        EVO_TOK_NEXT_OR_BREAK(tok, ',');
        assert(tok.value() == "2");

        // Field 3
        EVO_TOK_NEXT_OR_BREAK(tok, ',');
        assert(tok.value() == "3");

        // Field 4
        EVO_TOK_NEXT_OR_BREAK(tok, ',');
        assert(false);

        break;
    }

    return 0;
}
\endcode

When used with \link evo::StrTokLine StrTokLine\endlink, leave the `DELIM` argument empty.

\code{.cpp}
#include <evo/strtok.h>
using namespace evo;

int main() {
    StrTokLine tok("1\n2");
    for (;;) {
        // Line 1
        EVO_TOK_NEXT_OR_BREAK(tok,);
        assert(tok.value() == "1");

        // Line 2
        EVO_TOK_NEXT_OR_BREAK(tok,);
        assert(tok.value() == "2");

        break;
    }

    return 0;
}
\endcode
*/
#define EVO_TOK_NEXT_OR_BREAK(TOK, DELIM) if (!TOK.next(DELIM)) break

/** Helper for tokenizing using a break-loop.
 - A break-loop is a loop that always breaks at the end so doesn't actually loop, but also allows an early break to skip remaining code in the loop
 - This is similar to EVO_TOK_NEXT_OR_BREAK(), but allows using a more generic expression
 .

\par Example

These examples use `assert()` to represent field validation.

\code{.cpp}
#include <evo/strtok.h>
using namespace evo;

int main() {
    StrTok tok("1,'2a,2b',3");
    for (;;) {
        // Field 1
        EVO_TOK_OR_BREAK(tok.nextq(','));
        assert(tok.value() == "1");

        // Field 2
        EVO_TOK_OR_BREAK(tok.nextq(','));
        assert(tok.value() == "2a,2b");

        // Field 3
        EVO_TOK_OR_BREAK(tok.nextq(','));
        assert(tok.value() == "3");

        // Field 4
        EVO_TOK_OR_BREAK(tok.nextq(','));
        assert(false);

        break;
    }

    return 0;
}
\endcode
*/
#define EVO_TOK_OR_BREAK(EXPR) if (!EXPR) break

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
