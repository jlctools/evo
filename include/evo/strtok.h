// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file strtok.h Evo string tokenizer. */
#pragma once
#ifndef INCL_evo_strtok_h
#define INCL_evo_strtok_h

// Includes
#include "substring.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoTokenizers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** Base tokenizer class -- see StrTok and StrTokR. */
class StrTokBase
{
public:
    typedef StrTokBase      BaseType;    ///< Base type
    typedef SubString::Size Size;        ///< %String size type

    /** Get current index before next token.
     \return  Current index, END if at end
     */
    inline Size index() const
        { return index_; }

    /** Get current delimiter before next token.
     \return  Current delimiter, null if none or at end
     */
    inline Char delim() const
        { return delim_; }

    /** Get current token value from last call to next().
     \return  Current token value
    */
    inline const SubString& value() const
        { return value_; }

protected:
    SubString string_;        ///< %String being tokenized, NULL for none
    Size      index_;        ///< Current index, END when at end
    Char      delim_;        ///< Current delimiter, null when none or at end
    SubString value_;        ///< Current value

    /** Default constructor creates empty tokenizer. */
    inline StrTokBase()
        { index_ = END; }

    /** Default constructor creates empty tokenizer. */
    inline StrTokBase(const SubString& string) :
        string_( string ),
        index_(  END )
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTokBase(const BaseType& src) :
        string_ ( src.string_ ),
        index_(   src.index_ ),
        delim_(   src.delim_ ),
        value_(   src.value_ )
        { }

    /** Copy data. */
    inline void copy(const BaseType& src) {
        this->string_ = src.string_;
        this->index_  = src.index_;
        this->delim_  = src.delim_;
        this->value_  = src.value_;
    }
};

///////////////////////////////////////////////////////////////////////////////

// Implementation - Temp Macros
/** \cond impl */
#define EVO_TMP_STRTOK_RESET \
    const char* str_data_ = this->string_.data_; \
    Size        str_size_ = this->string_.size_; \
    Size&       ind_ =      this->index_; \
    char ch; \
    ind_ = 0; \
    while ( ind_ < str_size_ && ((ch=str_data_[ind_]) == ' ' || ch == '\t') ) \
        ++ind_; \
    if (ind_ >= str_size_) \
        ind_ = END;
/** \endcond */

/** %String forward tokenizer.
 - Variants: \link evo::StrTokWord StrTokWord\endlink
 - This skips whitespace between delimiters so tokens will not start or end with any whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For reverse tokenizing see: StrTokR
 - For "strict" tokenizing (without skipping whitespace) see: StrTokS, StrTokRS
 - Note: All tokenizers (including forward and reverse) implement the exact same interface
 .
Example:
\code
String str = "one, two, three";

// Tokens:
//  one
//  two
//  three
{
    StrTok tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
*/
class StrTok : public StrTokBase
{
public:
    typedef StrTok          ThisType;    ///< This type
    typedef StrTokBase      BaseType;    ///< Base type
    typedef SubString::Size Size;        ///< %String size type

    /** Default constructor creates empty tokenizer. */
    inline StrTok()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTok(const ThisType& src) : StrTokBase((const BaseType&)src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTok(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string. Call next() or nextw() for each token.
     \param  str  %String to tokenize
    */
    inline StrTok(const SubString& str) : StrTokBase(str)
        { EVO_TMP_STRTOK_RESET; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from beginning. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    inline ThisType& operator=(const SubString& str) {
        this->string_ = str;
        this->value_.set();
        this->delim_.set();
        EVO_TMP_STRTOK_RESET;
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    inline ThisType& reset() {
        this->delim_.set();
        this->value_.set();
        EVO_TMP_STRTOK_RESET
        return *this;
    }

    /** Find next token using delimiter. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool next(char delim) {
        const Size  str_size_ = this->string_.size_;
        Size&       ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ < str_size_ && ((ch=str_data_[ind_]) == ' ' || ch == '\t') )
            ++ind_;

        Size start = ind_, end = ind_;
        while (ind_ < str_size_) {
            switch (ch=str_data_[ind_]) {
                case ' ':
                case '\t':
                    ++ind_;
                    break;
                default:
                    if (ch == delim) {
                        ++ind_;
                        this->value_.set2(this->string_, start, end);
                        this->delim_ = delim;
                        return true;
                    }
                    end = ++ind_;
                    break;
            }
        }
        ind_ = END;
        this->value_.set2(this->string_, start, end);
        this->delim_.set();
        return true;
    }

    /** Find next token with quoting support using delimiter with quoting support. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextq(char delim) {
        const char CHAR_DEL = '\x7F';
        const Size str_size_ = this->string_.size_;
        Size&      ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        // Skip whitespace
        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ < str_size_ && ((ch=str_data_[ind_]) == ' ' || ch == '\t') )
            ++ind_;

        // Check for quoting
        char quote_char  = 0;
        char quote_count = 1;
        if ( ind_ < str_size_ && ((ch=str_data_[ind_]) == '\'' || ch == '"' || ch == '`') ) {
            quote_char = ch;
            if (ind_+2 < str_size_ && str_data_[ind_+1] == quote_char && str_data_[ind_+2] == quote_char)
                quote_count = 3;
            else if (quote_char == '`' && ind_+1 < str_size_ && str_data_[ind_+1] == CHAR_DEL)
                quote_count = 2;
            ind_ += quote_count;
        }

        // Parse next token
        bool end_quoted = false;
        Size start = ind_, end_unquoted = END, ind_unquoted = 0;
        Size end = ind_;
        while (ind_ < str_size_) {
            switch (ch=str_data_[ind_]) {
                case ' ':
                case '\t':
                    // Whitespace -- moves index but not end
                    ++ind_;
                    break;
                default:
                    if (ch == quote_char && ch != 0) {
                        // Found end-quote
                        if (quote_count == 3) {
                            if (ind_+2 < str_size_ && str_data_[ind_+1] == quote_char && str_data_[ind_+2] == quote_char) {
                                // Any extra quotes before end-triple-quote are part of token
                                end_quoted = true;
                                end = ind_;
                                ind_ += quote_count;
                                while (ind_ < str_size_ && str_data_[ind_] == quote_char)
                                    ++ind_, ++end;
                            } else
                                // Not an end-triple-quote, include in token, update end
                                end = ++ind_;
                        } else if (quote_count == 2) {
                            // Backtick-DEL
                            if (ind_+1 < str_size_ && str_data_[ind_+1] == CHAR_DEL) {
                                end_quoted = true;
                                end = ind_;
                                ind_ += quote_count;
                            } else
                                // Not an end Backtick-DEL, include in token, update end
                                end = ++ind_;
                        } else {
                            end_quoted = true;
                            end        = ind_;
                            ind_ += quote_count;
                        }
                    } else if (ch == delim) {
                        // Found delimiter
                        if (!quote_char || end_quoted) {
                            ++ind_;
                            this->value_.set2(this->string_, start, end);
                            this->delim_ = delim;
                            return true;
                        }
                        if (end_unquoted == END) {
                            // Skipping delim due to quoting, save state for later in case of quoting error
                            end_unquoted = end;
                            ind_unquoted = ind_;
                        }
                        end = ++ind_;
                    } else {
                        // Include char in token, update end
                        end = ++ind_;
                        if (end_quoted) {
                            // Data after end-quote, revert to unquoted
                            start -= quote_count;
                            if (end_unquoted != END) {
                                // Use delim that was skipped due to quoting
                                end  = end_unquoted;
                                ind_ = ind_unquoted + 1;
                                this->value_.set2(this->string_, start, end);
                                this->delim_ = delim;
                                return true;
                            }
                            end_quoted = false;
                            quote_char = 0;
                        }
                    }
                    break;
            }
        }

        if (quote_char != 0 && !end_quoted) {
            // Missing end-quote, revert to unquoted
            if (end_unquoted != END) {
                // Use delim that was skipped due to quoting
                end  = end_unquoted;
                ind_ = ind_unquoted + 1;
                this->delim_ = delim;
            } else {
                ind_ = END;
                this->delim_.set();
            }
            start -= quote_count;
        } else {
            ind_ = END;
            this->delim_.set();
        }
        this->value_.set2(this->string_, start, end);
        return true;
    }

    /** Find next token using word delimiter. Call value() to get token value.
     - Same as next(char) except this will also skip any leading extra delimiters before next token
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool nextw(char delim) {
        const Size  str_size_ = this->string_.size_;
        Size&       ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ < str_size_ && ((ch=str_data_[ind_]) == ' ' || ch == '\t' || ch == delim) )
            ++ind_;

        if (ind_ == str_size_) {
            ind_ = END;
            this->value_.set();
            this->delim_.set();
            return false;
        } else {
            Size start = ind_, end = ind_;
            while (ind_ < str_size_) {
                switch (ch=str_data_[ind_]) {
                    case ' ':
                    case '\t':
                        ++ind_;
                        break;
                    default:
                        if (ch == delim) {
                            ++ind_;
                            this->value_.set2(this->string_, start, end);
                            this->delim_ = ch;
                            return true;
                        }
                        end = ++ind_;
                        break;
                }
            }
            ind_ = END;
            this->value_.set2(this->string_, start, end);
            this->delim_.set();
            return true;
        }
    }

    /** Find next token using any of given delimiters. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     .
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const SubString& delims) {
        const Size  str_size_ = this->string_.size_;
        Size&       ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ < str_size_ && ((ch=str_data_[ind_]) == ' ' || ch == '\t') )
            ++ind_;

        Size start = ind_, end = ind_, i;
        const Size        delim_count = delims.size();
        const char* const delim_data  = delims.data();
        while (ind_ < str_size_) {
            switch (ch=str_data_[ind_]) {
                case ' ':
                case '\t':
                    ++ind_;
                    break;
                default:
                    for (i=0; i<delim_count; ++i) {
                        if (ch == delim_data[i]) {
                            ++ind_;
                            this->value_.set2(this->string_, start, end);
                            this->delim_ = ch;
                            return true;
                        }
                    }
                    end = ++ind_;
                    break;
            }
        }
        ind_ = END;
        this->value_.set2(this->string_, start, end);
        this->delim_.set();
        return true;
    }

    /** Find next token using any of given delimiters with quoting support. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     .
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextanyq(const SubString& delims) {
        const char CHAR_DEL = '\x7F';
        const Size  str_size_ = this->string_.size_;
        Size&       ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        // Skip whitespace
        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ < str_size_ && ((ch=str_data_[ind_]) == ' ' || ch == '\t') )
            ++ind_;

        // Check for quoting
        char quote_char  = 0;
        char quote_count = 1;
        if ( ind_ < str_size_ && ((ch=str_data_[ind_]) == '\'' || ch == '"' || ch == '`') ) {
            quote_char = ch;
            if (ind_+2 < str_size_ && str_data_[ind_+1] == quote_char && str_data_[ind_+2] == quote_char)
                quote_count = 3;
            else if (quote_char == '`' && ind_+1 < str_size_ && str_data_[ind_+1] == CHAR_DEL)
                quote_count = 2;
            ind_ += quote_count;
        }

        // Parse next token
        bool end_quoted = false;
        Size start = ind_, end = ind_, end_unquoted = END, ind_unquoted = 0, i;
        const Size        delim_count = delims.size();
        const char* const delim_data  = delims.data();
        while (ind_ < str_size_) {
            switch (ch=str_data_[ind_]) {
                case ' ':
                case '\t':
                    ++ind_;
                    break;
                default:
                    if (ch == quote_char && ch != 0) {
                        // Found end-quote
                        if (quote_count == 3) {
                            if (ind_+2 < str_size_ && str_data_[ind_+1] == quote_char && str_data_[ind_+2] == quote_char) {
                                // Any extra quotes before end-triple-quote are part of token
                                end_quoted = true;
                                end = ind_;
                                ind_ += quote_count;
                                while (ind_ < str_size_ && str_data_[ind_] == quote_char)
                                    ++ind_, ++end;
                            } else
                                // Not an end-triple-quote, include in token, update end
                                end = ++ind_;
                        } else if (quote_count == 2) {
                            // Backtick-DEL
                            if (ind_+1 < str_size_ && str_data_[ind_+1] == CHAR_DEL) {
                                end_quoted = true;
                                end = ind_;
                                ind_ += quote_count;
                            } else
                                // Not an end Backtick-DEL, include in token, update end
                                end = ++ind_;
                        } else {
                            end_quoted = true;
                            end        = ind_;
                            ind_ += quote_count;
                        }
                    } else {
                        bool found_delim = false;
                        for (i=0; i<delim_count; ++i) {
                            if (ch == delim_data[i]) {
                                // Found delimiter
                                if (!quote_char || end_quoted) {
                                    ++ind_;
                                    this->value_.set2(this->string_, start, end);
                                    this->delim_ = ch;
                                    return true;
                                }
                                if (end_unquoted == END) {
                                    // Skipping delim due to quoting, save state for later in case of quoting error
                                    end_unquoted = end;
                                    ind_unquoted = ind_;
                                }
                                end = ++ind_;
                                found_delim = true;
                                break;
                            }
                        }
                        if (!found_delim) {
                            // Include char in token, update end
                            end = ++ind_;
                            if (end_quoted) {
                                // Data after end-quote, revert to unquoted
                                start -= quote_count;
                                if (end_unquoted != END) {
                                    // Use delim that was skipped due to quoting
                                    end  = end_unquoted;
                                    ind_ = ind_unquoted + 1;
                                    this->value_.set2(this->string_, start, end);
                                    this->delim_ = str_data_[ind_unquoted];
                                    return true;
                                }
                                end_quoted = false;
                                quote_char = 0;
                            }
                        }
                    }
                    break;
            }
        }

        if (quote_char != 0 && !end_quoted) {
            // Missing end-quote, revert to unquoted
            if (end_unquoted != END) {
                // Use delim that was skipped due to quoting
                end  = end_unquoted;
                ind_ = ind_unquoted + 1;
                this->delim_ = str_data_[ind_unquoted];
            } else {
                ind_ = END;
                this->delim_.set();
            }
            start -= quote_count;
        } else {
            ind_ = END;
            this->delim_.set();
        }
        this->value_.set2(this->string_, start, end);
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
    static inline typename C::Size split(C& items, const T& str, char delim=',') {
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
    static inline SubString splitat(const T& str, Size index, char delim=',') {
        SubString result;
        ThisType tok(str);
        for (Size i=0; tok.next(delim); ++i)
            if (i == index)
                { result = tok.value(); break; }
        return result;
    }
};

// Implementation - Temp Macros
#undef EVO_TMP_STRTOK_RESET

///////////////////////////////////////////////////////////////////////////////

// Implementation - Temp Macros
/** \cond impl */
#define EVO_TMP_STRTOK_RESET \
    const char* str_data_ = this->string_.data_; \
    Size&       ind_      = this->index_; \
    char ch; \
    ind_ = this->string_.size_; \
    while ( ind_ > 0 && ((ch=str_data_[ind_-1]) == ' ' || ch == '\t') ) \
        --ind_; \
    if (ind_ == 0) \
        ind_ = END;
/** \endcond */

/** %String reverse tokenizer.
 - Variants: \link evo::StrTokWordR StrTokWordR\endlink
 - This skips whitespace between delimiters so tokens will not start or end with any whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For forward tokenizing see: StrTok
 - For "strict" tokenizing (without skipping whitespace) see: StrTokRS, StrTokS
 - Note: All tokenizers (including forward and reverse) implement the exact same interface
 .
Example:
\code
String str = "one, two, three";

// Tokens:
//  three
//  two
//  one
{
    StrTokR tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
*/
class StrTokR : public StrTokBase
{
public:
    typedef StrTokR         ThisType;        ///< This type
    typedef StrTokBase      BaseType;        ///< Base type
    typedef SubString::Size Size;            ///< %String size type

    /** Default constructor creates empty tokenizer. */
    inline StrTokR() : StrTokBase()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTokR(const ThisType& src) : StrTokBase(src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTokR(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string. Call next() or nextw() for each token.
     \param  str  %String to tokenize
    */
    inline StrTokR(const SubString& str) : StrTokBase(str)
        { EVO_TMP_STRTOK_RESET; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from end. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    inline ThisType& operator=(const SubString& str) {
        this->string_ = str;
        this->value_.set();
        this->delim_.set();
        EVO_TMP_STRTOK_RESET;
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    inline ThisType& reset() {
        this->value_.set();
        this->delim_.set();
        EVO_TMP_STRTOK_RESET;
        return *this;
    }

    /** Find next token using delimiter (in reverse order). Call value() to get token value.
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    bool next(char delim) {
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ > 0 && ((ch=str_data_[ind_-1]) == ' ' || ch == '\t') )
            --ind_;

        Size start = ind_, end = ind_;
        while (ind_ > 0) {
            switch (ch=str_data_[ind_-1]) {
                case ' ':
                case '\t':
                    --ind_;
                    break;
                default:
                    if (ch == delim) {
                        this->value_.set2(this->string_, start, end);
                        this->delim_ = delim;
                        --ind_;
                        return true;
                    }
                    start = --ind_;
                    break;
            }
        }
        this->value_.set2(this->string_, start, end);
        this->delim_.set();
        ind_ = END;
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
        const char CHAR_DEL = '\x7F';
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        // Skip whitespace
        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ > 0 && ((ch=str_data_[ind_-1]) == ' ' || ch == '\t') )
            --ind_;

        // Check for quoting
        char quote_char  = 0;
        char quote_count = 1;
        if (ind_ > 0) {
            if ( (ch=str_data_[ind_-1]) == '\'' || ch == '"' || ch == '`' ) {
                quote_char = ch;
                if (ind_ > 2 && str_data_[ind_-2] == quote_char && str_data_[ind_-3] == quote_char)
                    quote_count = 3;
                ind_ -= quote_count;
            } else if (ind_ > 3 && ch == CHAR_DEL && str_data_[ind_-2] == '`') {
                quote_char  = CHAR_DEL;
                quote_count = 2;
                ind_ -= 2;
            }
        }

        // Parse next token
        bool quoting_valid = false;
        Size start = ind_;
        Size end = ind_, start_unquoted = END, ind_unquoted = 0;
        while (ind_ > 0) {
            switch (ch=str_data_[ind_-1]) {
                case ' ':
                case '\t':
                    // Whitespace -- moves index but not start
                    --ind_;
                    break;
                default:
                    if (ch == quote_char && ch != 0) {
                        // Found begin-quote
                        if (quote_count == 3) {
                            if (ind_ > 2 && str_data_[ind_-2] == quote_char && str_data_[ind_-3] == quote_char) {
                                // Any extra quotes after begin-triple-quote are part of token
                                quoting_valid = true;
                                start = ind_;
                                ind_ -= quote_count;
                                while (ind_ > 0 && str_data_[ind_-1] == quote_char)
                                    --ind_, --start;
                            } else
                                // Not a begin-triple-quote, include in token, update start
                                start = --ind_;
                        } else if (quote_count == 2) {
                            // Backtick-DEL
                            if (ind_ > 1 && str_data_[ind_-2] == '`') {
                                quoting_valid = true;
                                start = ind_;
                                ind_ -= quote_count;
                            } else
                                // Not a begin Backtick-DEL, include in token, update start
                                start = --ind_;
                        } else {
                            quoting_valid = true;
                            start         = ind_;
                            ind_ -= quote_count;
                        }
                    } else if (ch == delim) {
                        // Found delimiter
                        if (!quote_char || quoting_valid) {
                            --ind_;
                            this->value_.set2(this->string_, start, end);
                            this->delim_ = delim;
                            return true;
                        }
                        if (start_unquoted == END) {
                            // Skipping delim due to quoting, save state for later in case of quoting error
                            start_unquoted = start;
                            ind_unquoted   = ind_ - 1;
                        }
                        start = --ind_;
                    } else {
                        // Include char in token, update start
                        start = --ind_;
                        if (quoting_valid) {
                            // Data before begin-quote, revert to unquoted
                            end += quote_count;
                            if (start_unquoted != END) {
                                // Use delim that was skipped due to quoting
                                start = start_unquoted;
                                ind_  = ind_unquoted;
                                this->value_.set2(this->string_, start, end);
                                this->delim_ = delim;
                                return true;
                            }
                            quoting_valid = false;
                            quote_char    = 0;
                        }
                    }
                    break;
            }
        }

        if (quote_char != 0 && !quoting_valid) {
            // Missing end-quote, revert to unquoted
            if (start_unquoted != END) {
                // Use delim that was skipped due to quoting
                start  = start_unquoted;
                ind_   = ind_unquoted;
                this->delim_ = delim;
            } else {
                ind_ = END;
                this->delim_.set();
            }
            end += quote_count;
        } else {
            ind_ = END;
            this->delim_.set();
        }
        this->value_.set2(this->string_, start, end);
        return true;
    }

    /** Find next token using word delimiter (in reverse order). Call value() to get token value.
     - Same as next(char) except duplicate delimiters are skipped
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    inline bool nextw(char delim) {
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ > 0 && ((ch=str_data_[ind_-1]) == ' ' || ch == '\t' || ch == delim) )
            --ind_;

        if (ind_ == 0) {
            this->value_.set();
            this->delim_.set();
            ind_ = END;
            return false;
        } else {
            Size start = ind_, end = ind_;
            while (ind_ > 0) {
                switch (ch=str_data_[ind_-1]) {
                    case ' ':
                    case '\t':
                        --ind_;
                        break;
                    default:
                        if (ch == delim) {
                            this->value_.set2(this->string_, start, end);
                            this->delim_ = delim;
                            --ind_;
                            return true;
                        }
                        start = --ind_;
                        break;
                }
            }
            this->value_.set2(this->string_, start, end);
            this->delim_.set();
            ind_ = END;
            return true;
        }
    }

    /** Find next token using any of given delimiters (in reverse order). Call value() to get token value.
     - This will skip leading whitespace before and after next token
     .
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const SubString& delims) {
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ > 0 && ((ch=str_data_[ind_-1]) == ' ' || ch == '\t') )
            --ind_;

        Size end = ind_;
        Size start = ind_, i;
        const Size        delim_count = delims.size();
        const char* const delim_data  = delims.data();
        while (ind_ > 0) {
            switch (ch=str_data_[ind_-1]) {
                case ' ':
                case '\t':
                    --ind_;
                    break;
                default:
                    for (i=0; i<delim_count; ++i) {
                        if (ch == delim_data[i]) {
                            --ind_;
                            this->value_.set2(this->string_, start, end);
                            this->delim_ = ch;
                            return true;
                        }
                    }
                    start = --ind_;
                    break;
            }
        }
        this->value_.set2(this->string_, start, end);
        this->delim_.set();
        ind_ = END;
        return true;
    }

    /** Find next token using any of given delimiters (in reverse order) with quoting support. Call value() to get token value.
     - This will skip leading whitespace before and after next token
     - Token may be single-quoted ( ' ), double-quoted ( " ), backtick-quoted ( ` ), or triple-quoted ( ''' or """ or ``` )
       - This also supports backtick-DEL quoting -- backtick followed by the DEL char (7F) -- used when no other quoting is possible
     - Token is only considered quoted if it begins and ends with given quotes, after excluding whitespace -- so an unquoted token can contain quote chars
     .
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextanyq(const SubString& delims) {
        const char CHAR_DEL = '\x7F';
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        // Skip whitespace
        const char* str_data_ = this->string_.data_;
        char ch;
        while ( ind_ > 0 && ((ch=str_data_[ind_-1]) == ' ' || ch == '\t') )
            --ind_;

        // Check for quoting
        char quote_char  = 0;
        char quote_count = 1;
        if (ind_ > 0) {
            if ( (ch=str_data_[ind_-1]) == '\'' || ch == '"' || ch == '`' ) {
                quote_char = ch;
                if (ind_ > 2 && str_data_[ind_-2] == quote_char && str_data_[ind_-3] == quote_char)
                    quote_count = 3;
                ind_ -= quote_count;
            } else if (ind_ > 3 && ch == CHAR_DEL && str_data_[ind_-2] == '`') {
                quote_char  = CHAR_DEL;
                quote_count = 2;
                ind_ -= 2;
            }
        }

        // Parse next token
        bool quoting_valid = false;
        Size start = ind_, end = ind_, start_unquoted = END, ind_unquoted = 0, i;
        const Size        delim_count = delims.size();
        const char* const delim_data  = delims.data();
        while (ind_ > 0) {
            switch (ch=str_data_[ind_-1]) {
                case ' ':
                case '\t':
                    --ind_;
                    break;
                default:
                    if (ch == quote_char && ch != 0) {
                        // Found begin-quote
                        if (quote_count == 3) {
                            if (ind_ > 2 && str_data_[ind_-2] == quote_char && str_data_[ind_-3] == quote_char) {
                                // Any extra quotes after begin-triple-quote are part of token
                                quoting_valid = true;
                                start = ind_;
                                ind_ -= quote_count;
                                while (ind_ > 0 && str_data_[ind_-1] == quote_char)
                                    --ind_, --start;
                            } else
                                // Not a begin-triple-quote, include in token, update start
                                start = --ind_;
                        } else if (quote_count == 2) {
                            // Backtick-DEL
                            if (ind_ > 1 && str_data_[ind_-2] == '`') {
                                quoting_valid = true;
                                start = ind_;
                                ind_ -= quote_count;
                            } else
                                // Not a begin Backtick-DEL, include in token, update start
                                start = --ind_;
                        } else {
                            quoting_valid = true;
                            start         = ind_;
                            ind_ -= quote_count;
                        }
                    } else {
                        bool found_delim = false;
                        for (i=0; i<delim_count; ++i) {
                            if (ch == delim_data[i]) {
                                // Found delimiter
                                if (!quote_char || quoting_valid) {
                                    --ind_;
                                    this->value_.set2(this->string_, start, end);
                                    this->delim_ = ch;
                                    return true;
                                }
                                if (start_unquoted == END) {
                                    // Skipping delim due to quoting, save state for later in case of quoting error
                                    start_unquoted = start;
                                    ind_unquoted   = ind_ - 1;
                                }
                                start = --ind_;
                                found_delim = true;
                                break;
                            }
                        }
                        if (!found_delim) {
                            // Include char in token, update start
                            start = --ind_;
                            if (quoting_valid) {
                                // Data before begin-quote, revert to unquoted
                                end += quote_count;
                                if (start_unquoted != END) {
                                    // Use delim that was skipped due to quoting
                                    start = start_unquoted;
                                    ind_  = ind_unquoted;
                                    this->value_.set2(this->string_, start, end);
                                    this->delim_ = str_data_[ind_unquoted];
                                    return true;
                                }
                                quoting_valid = false;
                                quote_char    = 0;
                            }
                        }
                    }
                    break;
            }
        }

        if (quote_char != 0 && !quoting_valid) {
            // Missing end-quote, revert to unquoted
            if (start_unquoted != END) {
                // Use delim that was skipped due to quoting
                start  = start_unquoted;
                ind_   = ind_unquoted;
                this->delim_ = str_data_[ind_unquoted];
            } else {
                ind_ = END;
                this->delim_.set();
            }
            end += quote_count;
        } else {
            ind_ = END;
            this->delim_.set();
        }
        this->value_.set2(this->string_, start, end);
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
    static inline typename C::Size split(C& items, const T& str, char delim=',') {
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
    static inline SubString splitat(const T& str, Size index, char delim=',') {
        SubString result;
        ThisType tok(str);
        for (Size i=0; tok.next(delim); ++i)
            if (i == index)
                { result = tok.value(); break; }
        return result;
    }
};

// Implementation - Temp Macros
#undef EVO_TMP_STRTOK_RESET

///////////////////////////////////////////////////////////////////////////////

/** %String forward tokenizer (strict).
 - Variants: \link evo::StrTokWordS StrTokWordS\endlink
 - This does not skip whitespace so tokens may start or end with whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For reverse "strict" tokenizing see: StrTokRS
 - For "non-strict" tokenizing (skipping whitespace) see: StrTok, StrTokR
 - Note: All tokenizers (including forward and reverse) implement the exact same interface
 .
Example:
\code
String str = "one,two,three";

// Tokens:
//  one
//  two
//  three
{
    StrTokS tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
*/
class StrTokS : public StrTokBase
{
public:
    typedef StrTokS         ThisType;        ///< This type
    typedef StrTokBase      BaseType;        ///< Base type
    typedef SubString::Size Size;            ///< %String size type

    /** Default constructor creates empty tokenizer. */
    inline StrTokS()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTokS(const ThisType& src) : StrTokBase(src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTokS(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string. Call next() or nextw() for each token.
     \param  str  %String to tokenize
    */
    inline StrTokS(const SubString& str) : StrTokBase(str)
        { this->index_ = (this->string_.size_ > 0 ? 0 : (Size)END); }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from beginning. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    inline ThisType& operator=(const SubString& str) {
        this->string_ = str;
        this->value_.set();
        this->delim_.set();
        this->index_ = (this->string_.size_ > 0 ? 0 : (Size)END);
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    inline ThisType& reset() {
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
        const Size  str_size_ = this->string_.size_;
        Size&       ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        Size start = ind_;
        while (ind_ < str_size_) {
            if (str_data_[ind_] == delim) {
                this->value_.set2(this->string_, start, ind_);
                this->delim_ = delim;
                ++ind_;
                return true;
            }
            ++ind_;
        }
        this->value_.set(this->string_, start);
        this->delim_.set();
        ind_ = END;
        return true;
    }

    /** Find next token using word delimiter. Call value() to get token value.
     - Same as next(char) except duplicate delimiters are skipped
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    inline bool nextw(char delim) {
        const Size  str_size_ = this->string_.size_;
        Size&       ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        while (ind_ < str_size_ && str_data_[ind_] == delim)
            ++ind_;

        if (ind_ == str_size_) {
            ind_ = END;
            this->value_.set();
            this->delim_.set();
            return false;
        } else {
            Size start = ind_;
            while (ind_ < str_size_) {
                if (str_data_[ind_] == delim) {
                    this->value_.set2(this->string_, start, ind_);
                    this->delim_ = delim;
                    ++ind_;
                    return true;
                }
                ++ind_;
            }
            this->value_.set(this->string_, start);
            this->delim_.set();
            ind_ = END;
            return true;
        }
    }

    /** Find next token using any of given delimiters. Call value() to get token value.
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const SubString& delims) {
        const Size  str_size_ = this->string_.size_;
        Size&       ind_      = this->index_;
        if (ind_ > str_size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        Size start = ind_, i;
        const Size        delim_count = delims.size();
        const char* const delim_data  = delims.data();
        while (ind_ < str_size_) {
            ch = str_data_[ind_];
            for (i=0; i<delim_count; ++i) {
                if (ch == delim_data[i]) {
                    this->value_.set2(this->string_, start, ind_);
                    this->delim_ = ch;
                    ++ind_;
                    return true;
                }
            }
            ++ind_;
        }
        ind_ = END;
        this->value_.set(this->string_, start);
        this->delim_.set();
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
    static inline typename C::Size split(C& items, const T& str, char delim=',') {
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
    static inline SubString splitat(const T& str, Size index, char delim=',') {
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
 - Variants: \link evo::StrTokWordRS StrTokWordRS\endlink
 - This does not skip whitespace so tokens may start or end with whitespace
 - This references target string data -- results are undefined if target string is modified while referenced
 - For forward "strict" tokenizing see: StrTokS
 - For "non-strict" tokenizing (skipping whitespace) see: StrTokR, StrTok
 - Note: All tokenizers (including forward and reverse) implement the exact same interface
 .
Example:
\code
String str = "one,two,three";

// Tokens:
//  three
//  two
//  one
{
    StrTokRS tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
*/
class StrTokRS : public StrTokBase
{
public:
    typedef StrTokRS ThisType;        ///< This type
    typedef StrTokBase      BaseType;        ///< Base type
    typedef SubString::Size Size;            ///< %String size type

    /** Default constructor creates empty tokenizer. */
    inline StrTokRS() : StrTokBase()
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTokRS(const ThisType& src) : StrTokBase(src)
        { }

    /** Copy constructor.
     \param  src  Data to copy
    */
    inline StrTokRS(const BaseType& src) : StrTokBase(src)
        { }

    /** Constructor to start tokenizing given string. Call next() or nextw() for each token.
     \param  str  %String to tokenize
    */
    inline StrTokRS(const SubString& str) : StrTokBase(str)
        { this->index_ = (str.size_ > 0 ? str.size_ : END); }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }

    /** Assignment/Copy operator.
     \param  src  Data to copy
     \return      This
    */
    inline ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }

    /** Assignment operator to start tokenizing given string from end. Call next() or nextw() for each token.
     \param  str  %String to tokenize
     \return      This
    */
    inline ThisType& operator=(const SubString& str) {
        this->string_ = str;
        this->index_  = (str.size_ > 0 ? str.size_ : END);
        this->value_.set();
        this->delim_.set();
        return *this;
    }

    /** Reset to tokenize from beginning of string.
     \return  This
     */
    inline ThisType& reset() {
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
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        Size start = ind_;
        while (ind_ > 0) {
            --ind_;
            if (str_data_[ind_] == delim) {
                this->value_.set2(this->string_, ind_+1, start);
                this->delim_ = delim;
                return true;
            }
        }
        this->value_.set2(this->string_, 0, start);
        this->delim_.set();
        ind_ = END;
        return true;
    }

    /** Find next token using word delimiter (in reverse order). Call value() to get token value.
     - Same as next(char) except duplicate delimiters are skipped
     .
     \param  delim  Delimiter to use
     \return        Whether next token was found, false if no more
    */
    inline bool nextw(char delim) {
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        while (ind_ > 0 && str_data_[ind_-1] == delim)
            --ind_;

        if (ind_ == 0) {
            this->value_.set();
            this->delim_.set();
            ind_ = END;
            return false;
        } else {
            Size start = ind_;
            while (ind_ > 0) {
                --ind_;
                if (str_data_[ind_] == delim) {
                    this->value_.set2(this->string_, ind_+1, start);
                    this->delim_ = delim;
                    return true;
                }
            }
            this->value_.set2(this->string_, 0, start);
            this->delim_.set();
            ind_ = END;
            return true;
        }
    }

    /** Find next token using any of given delimiters (in reverse order). Call value() to get token value.
     \param  delims  Delimiters to use
     \return         Whether next token was found, false if no more
    */
    bool nextany(const SubString& delims) {
        Size& ind_ = this->index_;
        if (ind_ > this->string_.size_) {
            this->value_.set();
            return false;
        }

        const char* str_data_ = this->string_.data_;
        char ch;
        Size start = ind_, i;
        const Size        delim_count = delims.size();
        const char* const delim_data  = delims.data();
        while (ind_ > 0) {
            ch = str_data_[--ind_];
            for (i=0; i<delim_count; ++i) {
                if (ch == delim_data[i]) {
                    this->value_.set2(this->string_, ind_+1, start);
                    this->delim_ = ch;
                    return true;
                }
            }
        }
        this->value_.set2(this->string_, 0, start);
        this->delim_.set();
        ind_ = END;
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
    static inline typename C::Size split(C& items, const T& str, char delim=',') {
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
    static inline SubString splitat(const T& str, Size index, char delim=',') {
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
 - See \link evo::StrTokWord StrTokWord\endlink for example
 .
 \tparam  T        %String tokenizer to extend
 \tparam  NextCh   Pointer to member function for next() to call
 \tparam  NextAny  Pointer to member function for nextany() to call
*/
template<class T, bool (T::*NextCh)(char), bool (T::*NextAny)(const SubString&)=&T::nextany>
struct StrTokVariant : public T
{
    typedef StrTokVariant<T,NextCh,NextAny> ThisType;   ///< This type
    typedef StrTokBase                      BaseType;   ///< Root base type
    typedef SubString::Size                 Size;       ///< %String size type

    inline StrTokVariant()
        { }
    inline StrTokVariant(const ThisType& src) : T((const T&)src)
        { }
    inline StrTokVariant(const BaseType& src) : T(src)
        { }
    inline StrTokVariant(const SubString& str) : T(str)
        { }

    inline ThisType& operator=(const ThisType& src)
        { this->copy(src); return *this; }
    inline ThisType& operator=(const BaseType& src)
        { this->copy(src); return *this; }
    inline ThisType& operator=(const SubString& str)
        { T::operator=(str); return *this; }

    inline bool next(char delim)
        { return ( ((T*)this)->*NextCh )(delim); }
    inline bool nextany(const SubString& delims)
        { return ( ((T*)this)->*NextAny )(delims); }

    // TODO, needed?

    template<class C,class S>
    static inline typename C::Size split(C& items, const S& str, char delim=',') {
        typename C::Size count = 0;
        T tok(str);
        for (; (tok.*NextCh)(delim); ++count)
            items.add( tok.value().template convert<typename C::Item>() );
        return count;
    }

    template<class S>
    static inline SubString splitat(const S& str, Size index, char delim=',') {
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
String str = "one, \"two\", three";

// Tokens:
//  one
//  two
//  three
{
    StrTokQ tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
 */
typedef StrTokVariant<StrTok,&StrTok::nextq,&StrTok::nextanyq> StrTokQ;

/** %String reverse tokenizer based on StrTokR with quoted token support.
 - This is the same as StrTokR, except next() is overridden to behave as StrTokR::nextq()
 .
Example:
\code
String str = "one, \"two\", three";

// Tokens:
//  three
//  two
//  one
{
    StrTokQR tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
 */
typedef StrTokVariant<StrTokR,&StrTokR::nextq,&StrTokR::nextanyq> StrTokQR;

///////////////////////////////////////////////////////////////////////////////

/** %String forward word tokenizer based on StrTok.
 - This is the same as StrTok, except next() is overridden to behave as StrTok::nextw()
 .
Example:
\code
String str = "one, two, three";

// Tokens:
//  one
//  two
//  three
{
    StrTokWord tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
 */
typedef StrTokVariant<StrTok,&StrTok::nextw> StrTokWord;

/** %String reverse word tokenizer based on StrTokR.
 - This is the same as StrTokR, except next() is overridden to behave as StrTokR::nextw()
 .
Example:
\code
String str = "one, two, three";

// Tokens:
//  three
//  two
//  one
{
    StrTokWordR tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
 */
typedef StrTokVariant<StrTokR,&StrTokR::nextw> StrTokWordR;

/** %String forward word tokenizer based on StrTokS (strict).
 - This is the same as StrTokS, except next() is overridden to behave as StrTokS::nextw()
 .
Example:
\code
String str = "one,two,three";

// Tokens:
//  one
//  two
//  three
{
    StrTokWordS tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
 */
typedef StrTokVariant<StrTokS,&StrTokS::nextw> StrTokWordS;

/** %String reverse word tokenizer based on StrTokRS (strict).
 - This is the same as StrTokRS, except next() is overridden to behave as StrTokRS::nextw()
 .
Example:
\code
String str = "one,two,three";

// Tokens:
//  three
//  two
//  one
{
    StrTokWordRS tok(str);
    while (tok.next(','))
        cout << tok.value();
}
\endcode
 */
typedef StrTokVariant<StrTokRS,&StrTokRS::nextw> StrTokWordRS;

///////////////////////////////////////////////////////////////////////////////

//@}
} // Namespace: evo
#endif
