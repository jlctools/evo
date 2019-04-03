// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file file.h Evo File I/O stream class. */
#pragma once
#ifndef INCL_evo_file_h
#define INCL_evo_file_h

#include "iobase.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** %File I/O stream.
 - Use to read and write files
 - This is a Stream (read/write) using I/O device IoFile
 - This throws an ExceptionStream on error if exceptions are enabled, otherwise use error() to check for error
 - See also: Directory, FilePath

\par Methods

 - %File:
   - open()
     - isopen()
   - close()
   - seek()
   - pos()
 - Data:
   - readline(), readtext(), readbin()
     - bufread()
   - writeline(), writetext(), writebin()
     - operator<<()
     - flush()
     - bufwrite()
 - Error handling:
   - operator!()
   - error()
   - errormsg_out()

\par Example

\code
#include <evo/io.h>
#include <evo/file.h>
using namespace evo;
static Console& c = con();

int main() {
    const char* filename = "tmpfile.txt";

    // Write new file
    {
        File file(filename, oWRITE_NEW);
        file << "line one" << NL;
        file << "line " << 2 << NL;
        file << "line three" << NL;
    }

    // Read and print file
    {
        File file(filename);
        for (String line; file.readline(line); )
            c.out << "> " << line << NL;
    }

    return 0;
}
\endcode
*/
class File : public Stream<IoFile> {
public:
    typedef Stream<IoFile> Base;    ///< Base class alias

    /** Constructor.
     - This initializes without opening a file, use open() to open a file
     .
     \param  nl          Default newline value to use for text reads/writes
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    File(Newline nl=NL_SYS, bool exceptions=EVO_EXCEPTIONS) : Base(nl)
        { excep(exceptions); }

    /** Constructor to open file.
     - Throws ExceptionStreamOpen on error, if exceptions enabled
     - If exceptions disabled: Call error() to check for error
     .
     \param  path        File path to use
     \param  mode        Access mode to use
     \param  flushlines  Whether to flush text output on newlines (line buffering)
     \param  nl          Default newline value to use for text reads/writes
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    File(const char* path, Open mode=oREAD, bool flushlines=false, Newline nl=NL_SYS, bool exceptions=EVO_EXCEPTIONS) : Base(nl)
        { excep(exceptions); open(path, mode, flushlines); }

    /** Constructor to open file and set default newline.
     - Throws ExceptionStreamOpen on error, if exceptions enabled
     - If exceptions disabled: Call error() to check for error
     .
     \param  path        File path to use
     \param  mode        Access mode to use
     \param  nl          Default newline value to use for text reads/writes
     \param  exceptions  Whether to enable exceptions on error, default set by Evo config: EVO_EXCEPTIONS
    */
    File(const char* path, Open mode, Newline nl, bool exceptions=EVO_EXCEPTIONS) : Base(nl)
        { excep(exceptions); open(path, mode, false); }

    /** Open file for read and/or writing.
     - Current file is closed first
     - Throws ExceptionStreamOpen on error, if exceptions enabled
     .
     \param  path        File path to use
     \param  mode        Access mode to use
     \param  flushlines  Whether to flush text output on newlines (line buffering)
     \return             Whether successful, false on error -- call error() for error code
    */
    bool open(const char* path, Open mode=oREAD, bool flushlines=false) {
        const bool result = ((error_=device_.open(path, mode)) == ENone);
        if (result) {
            Base::init(mode, flushlines);
            owned_ = true;
        } else if (excep_ && error_ != ENone) {
            EVO_THROW_ERR(evo::ExceptionFileOpen, "File::open() failed", error_);
        }
        return result;
    }

    /** Get current file position.
     - Call error() to check error code
     .
     \return  File position on success, 0 on error
    */
    ulongl pos() {
        assert( bufrd_.curbuf_offset <= bufrd_.readbuf.used );
        if (savepos_ > 0)
            return device_.pos(error_) + bufwr_.used;
        return device_.pos(error_) - (bufrd_.readbuf.used-bufrd_.curbuf_offset) + (bufwr_.used);
    }

    /** Seek to file position.
     - Call error() to check error code
     .
     \param  offset  Offset from start to seek to
     \param  start   Start position to seek from
     \return         New file position after seek, 0 for beginning or error
    */
    ulongl seek(ulongl offset, Seek start=sBegin) {
        if (bufwr_.used > 0) {
            if (!flush())
                return 0;
            savepos_ = 0;
        }
        if (bufrd_.readbuf.used > 0)
            bufrd_.readbuf.used = bufrd_.curbuf_offset = 0;
        return device_.seek(error_, offset, start);
    }

private:
    File(const File&);
    File& operator=(const File&);
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
