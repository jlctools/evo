// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file fmt.h Evo extended output formatting helpers. */
#pragma once
#ifndef INCL_evo_fmt_h
#define INCL_evo_fmt_h

#include "substring.h"

namespace evo {
/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/** \cond impl */
namespace impl {
    template<class T> struct FmtFieldType {
        typedef typename T::FmtFieldType Type;
    };
    template<> struct FmtFieldType<String> {
        typedef FmtString Type;
    };
}
/** \endcond */

///////////////////////////////////////////////////////////////////////////////

/** Text table formatter.
 - Call `add_*()` methods to add columns
 - Call `column_*()` methods to custom current columns
 - Optionally use FmtTableAttribs to set column attributes -- espeically useful with fmt_table_nocache() to update minimum-column width before writing columns
 - Call fmt_table() or fmt_table_nocache() to pair the formatter with an output String/Stream and use operator<<() to write row/column values
 - See Tables in \ref AdditionalFormatting
 .

Output formats (see Type enum):
 - Plain text, with columns separated by double-space and aligned where possible
 - Markdown table using GitHub style
 .
*/
struct FmtTable {
    /** Formatting type to use. */
    enum Type {
        tTEXT,      ///< Basic text table using whitespace to line up columns
        tMARKDOWN   ///< Text table using markdown syntax (GitHub style)
    };

    /** %Column information. */
    struct Column {
        SubString name;     ///< %Column name
        FmtAlign align;     ///< %Column alignment type, default: fLEFT
        int width;          ///< %Column width, default: 0

        /** Constructor using default values. */
        Column() : align(fLEFT), width(0) {
        }

        /** Constructor.
         \param  name   %Column name
         \param  align  %Column alignment value
         \param  width  %Column width
        */
        Column(const SubString& name, FmtAlign align, int width=0) : name(name), align(align), width(width) {
        }

        /** Constructor.
         - Uses default alignment: fLEFT
         .
         \param  name   %Column name
         \param  width  %Column width
        */
        Column(const SubString& name, int width=0) : name(name), align(fLEFT), width(width) {
        }

        /** Copy constructor.
         \param  src  Source to copy
        */
        Column(const Column& src) : name(src.name), align(src.align), width(src.width) {
        }

        /** Assignment operator.
         \param  src  Source to copy
         \return      This
        */
        Column& operator=(const Column& src) {
            name = src.name;
            align = src.align;
            width = src.width;
            return *this;
        }
    };

    List<Column> columns;   ///< %Column information

    /** Constructor. */
    FmtTable() {
    }

    /** Constructor calling add_columns().
     \param  cols  Pointer to Column array to add from
     \param  size  Number of columns to add, 0 to stop on empty name
    */
    FmtTable(const Column* cols, SizeT size) {
        add_columns(cols, size);
    }

    /** Constructor calling add_columns().
     \param  names     Pointer to array of strings to use for column names
     \param  size      Number of columns to add, 0 to stop on empty name
     \param  minwidth  Minimum width to use for all added columns
     \param  align     Alignment to use for all added columns
     \return           This
    */
    FmtTable(const StringBase* names, SizeT size, int minwidth=0, FmtAlign align=fLEFT) {
        add_columns(names, size, minwidth, align);
    }

    /** Copy constructor.
     \param  src  Source to copy
    */
    FmtTable(const FmtTable& src) : columns(src.columns) {
    }

    /** Assignment operator.
     \param  src  Source to copy
     \return      This
    */
    FmtTable& operator=(const FmtTable& src) {
        columns = src.columns;
        return *this;
    }

    /** Add column to table.
     \param  column  %Column to add
     \return         This
    */
    FmtTable& add_column(const Column& column) {
        Column& newcol = *(columns.add(column).lastM());
        if (newcol.width < 0 || (SizeT)newcol.width < newcol.name.size())
            newcol.width = (int)newcol.name.size();
        return *this;
    }

    /** Add column to table.
     \param  name   %Column name
     \param  align  %Column alignment
     \param  width  %Column minimum width
     \return        This
    */
    FmtTable& add_column(const StringBase& name, FmtAlign align=fLEFT, int width=0) {
        if (width < 0 || (SizeT)width < name.size_)
            width = (int)name.size_;
        columns.add(Column(name, align, width));
        return *this;
    }

    /** Add column to table.
     \param  name   %Column name
     \param  width  %Column minimum width
     \return        This
    */
    FmtTable& add_column(const StringBase& name, int width) {
        if (width < 0 || (SizeT)width < name.size_)
            width = (int)name.size_;
        columns.add(Column(name, width));
        return *this;
    }

    /** Add multiple columns to table.
     \param  cols  Pointer to Column array to add from
     \param  size  Number of columns to add, 0 to stop on empty name
     \return       This
    */
    FmtTable& add_columns(const Column* cols, SizeT size) {
        if (cols != NULL) {
            if (size == 0) {
                for (SizeT i = 0; cols[i].name.size_ > 0; ++i)
                    add_column(cols[i]);
            } else {
                columns.reserve(size);
                for (SizeT i = 0; i < size; ++i)
                    add_column(cols[i]);
            }
        }
        return *this;
    }

    /** Add multiple columns to table.
     \param  names     Pointer to array of strings to use for column names
     \param  size      Number of columns to add, 0 to stop on empty name
     \param  minwidth  Minimum width to use for all added columns
     \param  align     Alignment to use for all added columns
     \return           This
    */
    FmtTable& add_columns(const StringBase* names, SizeT size, int minwidth=0, FmtAlign align=fLEFT) {
        if (names != NULL) {
            if (size == 0) {
                for (SizeT i = 0; names[i].size_ > 0; ++i)
                    add_column(names[i], align, minwidth);
            } else {
                columns.reserve(size);
                for (SizeT i = 0; i < size; ++i)
                    add_column(names[i], align, minwidth);
            }
        }
        return *this;
    }

    /** %Set align value for column at given index.
     - Ignored if index is out of range
     .
     \param  index  %Column index
     \param  align  Alignment value
     \return        This
    */
    FmtTable& column_align(SizeT index, FmtAlign align) {
        if (index < columns.size())
            columns(index).align = align;
        return *this;
    }

    /** Update minimum width for column at given index.
     - Ignored if index is out of range
     - This only grows the column width (only modified if new width is larger)
     .
     \param  index  %Column index
     \param  width  Minimum width to use
     \return        This
    */
    FmtTable& column_minsize(SizeT index, int width) {
        if (index < columns.size()) {
            int& col_width = columns(index).width;
            if (width > col_width)
                col_width = width;
        }
        return *this;
    }

    /** Reset column information to defaults.
     \return  This
    */
    FmtTable& column_reset() {
        for (List<Column>::IterM iter(columns); iter; ++iter) {
            Column& col = *iter;
            col.align = fLEFT;
            col.width = (int)col.name.size();
        }
        return *this;
    }

    /** Used to write formatted output (used internally).
     - This is normally not used directly
     - See fmt_table() or fmt_table_nocache()
     .
    */
    struct Writer {
        FmtTable& table;        ///< Table info used
        Type type;              ///< Formatting type
        SizeT column_index;     ///< Current column index
        SizeT rows;             ///< Number of rows written
        bool started;           ///< True when writing has started, false when finished

        /** Constructor.
         \param  table  Table info to use
         \param  type   Output type
        */
        Writer(FmtTable& table, Type type=tTEXT) : table(table), type(type), column_index(0), rows(0), started(false) {
        }

        /** Write header output.
         - Ignored if no columns exist
         .
         \tparam  T  %String or stream type to use, inferred from argument
         \param   out  %String or stream to write to
         \return       out
        */
        template<class T>
        T& write_header(T& out) {
            const SizeT sz = table.columns.size();
            if (sz > 0) {
                started = true;
                if (type == tMARKDOWN)
                    out.writebin("| ", 2);

                for (SizeT i = 0; i < sz; ++i) {
                    if (i > 0) {
                        if (type == tMARKDOWN)
                            out.writebin(" | ", 3);
                        else
                            out.writebin("  ", 2);
                    }
                    const Column& col = table.columns[i];
                    out << FmtString(col.name, col.align, col.width, ' ');
                }

                if (type == tMARKDOWN) {
                    out.writebin(" |", 2);
                    out << NL;
                    out.writebin("| ", 2);
                    for (SizeT i = 0; i < sz; ++i) {
                        if (i > 0)
                            out.writebin(" | ", 3);
                        const Column& col = table.columns[i];
                        switch (col.align) {
                            case fRIGHT:
                                out.writechar('-', table.columns[i].width - 1);
                                out.writechar(':');
                                break;
                            case fCENTER:
                                out.writechar(':');
                                out.writechar('-', table.columns[i].width - 2);
                                out.writechar(':');
                                break;
                            default:
                                out.writechar('-', table.columns[i].width);
                                break;
                        }
                    }
                    out.writebin(" |", 2);
                }
                out << NL;
            }
            return out;
        }

        /** Write next value.
         - Ignored if no columns exist
         .
         \tparam  T  %String or stream type to use, inferred from `out` argument
         \tparam  U  Type of value to format, inferred from `fmtval` -- must be one of: String, FmtString, FmtFieldNum, FmtFieldFloat
         \param   out     %String or stream to write to
         \param   fmtval  Value to format
         \return          out
        */
        template<class T, class U>
        T& write_value(T& out, const U& fmtval) {
            const SizeT col_sz = table.columns.size();
            if (started && col_sz > 0) {
                assert( column_index < table.columns.size() );
                const Column& col = table.columns[column_index];
                typename impl::FmtFieldType<U>::Type v(fmtval, col.align, col.width);
                if (type == tMARKDOWN) {
                    if (column_index == 0) {
                        out.writebin("| ", 2);
                        out << v;
                        ++column_index;
                    } else if (++column_index >= col_sz) {
                        ++rows;
                        column_index = 0;
                        out.writebin(" | ", 3);
                        out << v;
                        out.writebin(" |", 2);
                        out << NL;
                    } else {
                        out.writebin(" | ", 3);
                        out << v;
                    }
                } else {
                    if (column_index > 0)
                        out.writebin("  ", 2);
                    out << v;
                    if (++column_index >= col_sz) {
                        column_index = 0;
                        ++rows;
                        out << NL;
                    }
                }
            }
            return out;
        }

        /** Write end to current row.
         - Ignored if no columns exist
         .
         \tparam  T  %String or stream type to use, inferred from `out` argument
         \param   out  %String or stream to write to
         \return       out
        */
        template<class T>
        T& write_endrow(T& out) {
            if (started && column_index > 0) {
                if (type == tMARKDOWN) {
                    for (SizeT sz = table.columns.size(); column_index < sz; ++column_index) {
                        out.writebin(" | ", 3);
                        out.writechar(' ', table.columns[column_index].width);
                    }
                    out.writebin(" |", 2);
                }
                column_index = 0;
                ++rows;
                out << NL;
            }
            return out;
        }

        /** Finish writing table.
         - Ignored if no columns exist
         .
         \tparam  T  %String or stream type to use, inferred from `out` argument
         \param   out  %String or stream to write to
         \return       out
        */
        template<class T>
        T& write_finished(T& out) {
            if (started) {
                write_endrow(out);
                if (rows == 0 && table.columns.size() > 0) {
                    if (type == tMARKDOWN) {
                        out.writebin("| ", 2);
                        out.writechar(' ', table.columns[0].width);
                        for (SizeT i = 1, sz = table.columns.size(); i < sz; ++i) {
                            out.writebin(" | ", 3);
                            out.writechar(' ', table.columns[i].width);
                        }
                        out.writebin(" |", 2);
                    }
                    out << NL;
                }
                column_index = 0;
                rows = 0;
                started = false;
            }
            return out;
        }
    };

    /** Used to write formatted output with caching (used internally).
     - This is normally not used directly
     - This wraps Writer with a caching layer so final column widths can be determined
     - See fmt_table()
     .
    */
    struct CachedWriter {
        typedef List<String> StringList;    ///< List of String values

        Writer writer;              ///< Writer for actual writing
        List<StringList> rowcache;  ///< List of cached rows
        StringList* current_row;    ///< Pointer to current row, NULL if about to start a new row
        bool started;               ///< True when writing has started, false when finished

        /** \copydoc Writer::Writer() */
        CachedWriter(FmtTable& table, Type type=tTEXT) : writer(table, type), current_row(NULL) {
        }

        /** \copydoc Writer::write_header() */
        template<class T>
        T& write_header(T& out) {
            started = true;
            return out;
        }

        /** \copydoc Writer::write_value() */
        template<class T, class U>
        T& write_value(T& out, const U& fmtval) {
            const SizeT col_sz = writer.table.columns.size();
            if (started && col_sz > 0) {
                if (current_row == NULL) {
                    current_row = rowcache.addnew().lastM();
                    current_row->reserve(writer.table.columns.size());
                }
                const SizeT i = current_row->size();
                if (i >= col_sz) {
                    current_row = rowcache.addnew().lastM();
                    current_row->reserve(writer.table.columns.size());
                }

                String& val = *(current_row->addnew().lastM());
                val.clear() << fmtval;
                writer.table.column_minsize(i, (int)val.size());
            }
            return out;
        }

        /** \copydoc Writer::write_endrow() */
        template<class T>
        T& write_endrow(T& out) {
            if (started) {
                current_row = NULL;
                writer.write_endrow(out);
            }
            return out;
        }

        /** \copydoc Writer::write_finished() */
        template<class T>
        T& write_finished(T& out) {
            if (started) {
                writer.write_header(out);
                for (SizeT i = 0, i_sz = rowcache.size(); i < i_sz; ++i) {
                    const StringList& row = rowcache[i];
                    for (SizeT j = 0, j_sz = row.size(); j < j_sz; ++j)
                        writer.write_value(out, row[j]);
                    writer.write_endrow(out);
                }
                writer.write_finished(out);
                rowcache.clear();
                started = false;
            }
            return out;
        }
    };
};

///////////////////////////////////////////////////////////////////////////////

/** Helper for adjusting FmtTable column attributes.
 - This is paired with FmtTable and used to update column attributes
 - Use operator<<() to update and step through each column, using similar syntax to FmtTableOut:
   \code
    FmtTableAttribs(table) << 5 << fRIGHT << FmtSetField(fRIGHT, 5);
   \endcode
   - This accepts:
     - \link FmtAlign\endlink value, such as fLEFT
     - Width integer
     - FmtSetField for a combination of the above
 - %Column widths only grow -- ignored if new width is smaller
 - This may use multiple passes -- use `NL` to reset to first column (optional but preferred):
   \code
    FmtTableAttribs(table)
        << 1 << 2 << 3 << NL     // first pass
        << 5 << 6 << 7 << NL;    // second pass increases widths
   \endcode
 - Useful with fmt_table_nocache()
 - See \ref AdditionalFormatting
 .
*/
struct FmtTableAttribs {
    FmtTable& table;    ///< Paired table to update
    SizeT column_index; ///< Current column index

    /** Constructor.
     \param  table  Reference to table to update
    */
    FmtTableAttribs(FmtTable& table) : table(table), column_index(0) {
    }

    /** %Set alignment for current column and move to next column.
     \param  align  %Column alignment value
     \return        This
    */
    FmtTableAttribs& operator<<(FmtAlign align) {
        const SizeT col_sz = table.columns.size();
        if (col_sz > 0) {
            table.columns(column_index).align = align;
            if (++column_index >= col_sz)
                column_index = 0;
        }
        return *this;
    }

    /** Update minimum width of current column and move to next column.
     - This updates updates the column width if the new value is greater
     .
     \param  minwidth  New minimum width for column
     \return           This
    */
    FmtTableAttribs& operator<<(int minwidth) {
        const SizeT col_sz = table.columns.size();
        if (col_sz > 0) {
            int& width = table.columns(column_index).width;
            if (minwidth > width)
                width = minwidth;
            if (++column_index >= col_sz)
                column_index = 0;
        }
        return *this;
    }

    /** Update column alignment and minimum width together and move to next column.
     - This updates updates the column width if the new value is greater
     .
     \param  field  Field info to get align and minwidth from
     \return        This
    */
    FmtTableAttribs& operator<<(const FmtSetField& field) {
        const SizeT col_sz = table.columns.size();
        if (col_sz > 0) {
            FmtTable::Column& col = table.columns(column_index);
            col.align = field.align;
            int& width = col.width;
            if (field.width > width)
                width = field.width;
            if (++column_index >= col_sz)
                column_index = 0;
        }
        return *this;
    }

    /** Reset by moving back to first column.
     \return  This
    */
    FmtTableAttribs& operator<<(Newline) {
        column_index = 0;
        return *this;
    }
};

///////////////////////////////////////////////////////////////////////////////

/** Output interface for writing text table to stream/string.
 - This is used to write rows using a FmtTable instance with column information
   - This may update column widths stored in the associated FmtTable if they're too small
 - Use fmt_table() or fmt_table_nocache() to get an instance of this type paired with a given string or stream (using type deduction)
 - See \ref AdditionalFormatting
 .
 \tparam  T  String, Stream, or StreamOut object to write to
 \tparam  W  Writer to use, one of: FmtTable::CachedWriter, or FmtTable::Writer
*/
template<class T, class W=FmtTable::CachedWriter>
struct FmtTableOut {
    typedef FmtTableOut<T,W> This;  ///< This type
    W writer;   ///< Writer to use with FmtTable
    T& out;     ///< Paired output string/stream to write to

    /** Constructor.
     \param  out    Output string/stream to write to
     \param  table  FmtTable instance to use for column information
     \param  type   Output type to use -- see FmtTable::Type
    */
    FmtTableOut(T& out, FmtTable& table, FmtTable::Type type=FmtTable::tTEXT) : writer(table, type), out(out)
        { writer.write_header(out); }

    /** Destructor to finish writing table.
     - \b Caution: In some cases a stream can throw an exception (if enabled) during the flush so it's better to do flush explicitly with `operator<<(Flush)`
    */
    ~FmtTableOut()
        { writer.write_finished(out); }

    /** Write value for current column in current row and move to next column.
     \param  val  Value to write
     \return      This
    */
    This& operator<<(char val)
        { writer.write_value(out, FmtChar(val, 1)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(const FmtChar& val)
        { writer.write_value(out, val); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(const char* val)
        { writer.write_value(out, FmtString(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(const StringBase& val)
        { writer.write_value(out, FmtString(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(const FmtString& val)
        { writer.write_value(out, val); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(short val)
        { writer.write_value(out, FmtShort(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(int val)
        { writer.write_value(out, FmtInt(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(long val)
        { writer.write_value(out, FmtLong(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(longl val)
        { writer.write_value(out, FmtLongL(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(ushort val)
        { writer.write_value(out, FmtUShort(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(uint val)
        { writer.write_value(out, FmtUInt(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(ulong val)
        { writer.write_value(out, FmtULong(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(ulongl val)
        { writer.write_value(out, FmtULongL(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(float val)
        { writer.write_value(out, FmtFloat(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(double val)
        { writer.write_value(out, FmtFloatD(val)); return *this; }

    /** \copydoc operator<<(char) */
    This& operator<<(ldouble val)
        { writer.write_value(out, FmtFloatL(val)); return *this; }

    /** Write FmtIntT value for current column in current row and move to next column.
     \tparam  V  Type used with FmtIntT, inferred from argument
     \param  val  Value to write
     \return      This
    */
    template<class V>
    This& operator<<(const FmtIntT<V>& val)
        { writer.write_value(out, val); return *this; }

    /** Write FmtFloatT value for current column in current row and move to next column.
     \tparam  V  Type used with FmtFloatT, inferred from argument
     \param  val  Value to write
     \return      This
    */
    template<class V>
    This& operator<<(const FmtFloatT<V>& val)
        { writer.write_value(out, val); return *this; }

    /** Write IntegerT value for current column in current row and move to next column.
     - The output will be empty/blank if `val` is null
     .
     \tparam  V  Type used with IntegerT, inferred from argument
     \param  val  Value to write
     \return      This
    */
    template<class V>
    This& operator<<(const IntegerT<V>& val) {
        if (val.null())
            writer.write_value(out, String("", 0));
        else
            writer.write_value(out, FmtIntT<V>(*val));
        return *this;
    }

    /** Write FloatT value for current column in current row and move to next column.
     - The output will be empty/blank if `val` is null
     .
     \tparam  V  Type used with FloatT, inferred from argument
     \param  val  Value to write
     \return      This
    */
    template<class V>
    This& operator<<(const FloatT<V>& val) {
        if (val.null())
            writer.write_value(out, String("", 0));
        else
            writer.write_value(out, FmtFloatT<V>(*val));
        return *this;
    }

    /** End current row.
     - This may be used to end the row early (leaving empty/blank cells)
     - The will end automatically on the last column so this is optional, but preferred to be explicit
     - Pass argument as `NL`
     .
     \return  This
    */
    This& operator<<(Newline)
        { writer.write_endrow(out); return *this; }

    /** Flush any cached output.
     - \b Caution: In some cases a stream can throw an exception (if enabled) during the flush so it's better to do flush explicitly with this
     - Pass argument as `fFLUSH`
    */
    This& operator<<(Flush)
        { writer.write_finished(out); return *this; }
};

///////////////////////////////////////////////////////////////////////////////

/** Create cached FmtTableOut paired with given string/stream to write to.
 - Output is cached until either explicitly flushed or the returned object is destroyed
 - Caching is used to determine the column widths from row values -- this has some overhead so fmt_table_nocache() may be preferred for best performance
 - \b Caution: In some cases flushing to a stream can throw an exception (if enabled) so it's better to flush explicitly with `<< fFLUSH` then to let the destructor flush and possibly throw
 - See \ref AdditionalFormatting
 .
 \tparam  T  String/Stream type to use, inferred from `out` argument
 \param  out    Output string/stream to write to
 \param  table  FmtTable instance to use for column information
 \param  type   Output type to use -- see FmtTable::Type
 \return        FmtTableOut object to use
*/
template<class T>
inline FmtTableOut<T,FmtTable::CachedWriter> fmt_table(T& out, FmtTable& table, FmtTable::Type type=FmtTable::tTEXT) {
    return FmtTableOut<T,FmtTable::CachedWriter>(out, table, type);
}

/** Create cached FmtTableOut paired with given string/stream to write to.
 - Output is written as requested using column information stored in `table`
 - FmtTableAttribs is often used first to update column widths before using this to write output values
 - Longer output values will not be aligned correctly if the column information isn't up to date -- see FmtTableAttribs
 - See \ref AdditionalFormatting
 .
 \tparam  T  String/Stream type to use, inferred from `out` argument
 \param  out    Output string/stream to write to
 \param  table  FmtTable instance to use for column information
 \param  type   Output type to use -- see FmtTable::Type
 \return        FmtTableOut object to use
*/
template<class T>
inline FmtTableOut<T,FmtTable::Writer> fmt_table_nocache(T& out, FmtTable& table, FmtTable::Type type=FmtTable::tTEXT) {
    return FmtTableOut<T,FmtTable::Writer>(out, table, type);
}

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
