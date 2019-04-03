// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file filepath.h Evo file path operations. */
#pragma once
#ifndef INCL_evo_filepath_h
#define INCL_evo_filepath_h

#include "substring.h"

// Namespace: evo
namespace evo {
/** \addtogroup EvoIO */
//@{

///////////////////////////////////////////////////////////////////////////////

/** %File and directory path helpers.
 - This has helpers for parsing and joining file path components in various ways
 - See also: File, Directory, get_cwd(), get_abspath()
 .

\par Helpers

 - abs()
 - validate(), validate_filename()
 - normalize(), normalize_case()
 - drive()
 - dirpath()
 - filename(), filename_base(), filename_ext()
 - split_list()
   - split_drive()
   - split_dirpath()
   - split_filename()
   - split_all(SubString&,SubString&,SubString&,const SubString& path)
   - split_all(SubString&,SubString&,SubString&,SubString&,const SubString& path)
 - join()
   - join_list()
   - join_drive()
   - join_dirpath()
   - join_filename()
   - join_all(String&,const SubString&,const SubString&,const SubString&)
   - join_all(String&,const SubString&,const SubString&,const SubString&,const SubString&)
*/
class FilePath {
private:
#if defined(_WIN32)
    // Windows: Check for reserved filename: CON, PRN, AUX, NUL, COM0 - COM9, LPT0 - LPT9
    static bool reserved_filename(const char* name, StrSizeT size) {
        if (size > 4 && name[4] == '.')
            size = 4;
        else if (size > 3 && name[3] == '.')
            size = 3;
        char buf[3];
        switch (size) {
            case 3:
                buf[0] = ascii_tolower(name[0]);
                buf[1] = ascii_tolower(name[1]);
                buf[2] = ascii_tolower(name[2]);
                if ( (buf[0] == 'c' && buf[1] == 'o' && buf[2] == 'n') ||
                        (buf[0] == 'p' && buf[1] == 'r' && buf[2] == 'n') ||
                        (buf[0] == 'a' && buf[1] == 'u' && buf[2] == 'x') ||
                        (buf[0] == 'n' && buf[1] == 'u' && buf[2] == 'l') )
                    return true;                
                break;
            case 4:
                buf[0] = ascii_tolower(name[0]);
                buf[1] = ascii_tolower(name[1]);
                buf[2] = ascii_tolower(name[2]);
                if ( ( (buf[0] == 'c' && buf[1] == 'o' && buf[2] == 'm') ||
                       (buf[0] == 'l' && buf[1] == 'p' && buf[2] == 't') ) &&
                     name[3] >= '1' && name[3] <= '9' )
                    return true;                
                break;
            default:
                break;
        }
        return false;
    }

    // Windows: Path component name cannot end with space, and name other than '.' or '..' cannot end with a '.'
    static bool valid_filename(const char* name, StrSizeT size, bool strict) {
        const char lastch = name[size - 1];
        if (lastch == ' ' || (size > 1 && lastch == '.' && (size > 2 || *name != '.')))
            return false;
        return (!strict || !reserved_filename(name, size));
    }
#endif

public:
    static const char PATH_DELIM_WIN32 = '\\';  ///< Path delimiter char for Windows systems
    static const char PATH_DELIM_POSIX = '/';   ///< Path delimiter char for Linux/Unix systems

#if defined(_WIN32)
    static const char PATH_DELIM = PATH_DELIM_WIN32;
#else
    static const char PATH_DELIM = PATH_DELIM_POSIX;    ///< Path delimiter char for current OS
#endif

    /** Check whether path is an absolute path.
     - Linux/Unix: Path is absolute if it begins with a slash
     - Windows: Path is absolute if it begins with a backslash, or a drive letter followed by a colon then followed by a backslash (backslash optional if `strict=false`)
       - A network (UNC) path beginning with a double-backslash is considered an absolute path
       - Note that a drive letter without a backslash (ex: `c:foo\bar`) is relative to the current dir on that drive so is not absolute, unless `strict=false`
       - This recognizes both slashes and backslashes as path delimiters, though Windows itself only supports backslashes -- see normalize_case() to correct
     .
     \param  path    Path to check
     \param  strict  Whether to use strict mode (only used in Windows): false to consider a relative path with a drive as an absolute path (ex: `c:foo`)
     \return         Whether path is absolute
    */
    static bool abs(const SubString& path, bool strict=true) {
    #if defined(_WIN32)
        if (path.starts(PATH_DELIM) || path.starts(PATH_DELIM_POSIX))
            return true;
        if (strict)
            return (path.size() >= 3 && path[1] == ':' && (path[2] == PATH_DELIM || path[2] == PATH_DELIM_POSIX));
        return (path.size() >= 2 && path[1] == ':');
    #else
        return path.starts(PATH_DELIM);
    #endif
    }

    /** Check whether given path has a drive component.
     - Windows: Path is a drive when it starts with a drive letter followed by a colon, or if a network share (UNC) path that starts with a double-backslash or double slash
     - Linux/Unix: Always false
     .
     \param  path  Path to check
     \return       Whether `path` has a drive
    */
    static bool hasdrive(const SubString& path) {
    #if defined(_WIN32)
        return (path.starts("\\\\", 2) || path.starts("//", 2) || (path.size() >= 2 && path[1] == ':'));
    #else
        return false;
    #endif
    }

    /** Validate whether path is valid.
     - Path is invalid if it contains any invalid characters
     - Non-ASCII UTF-8 characters are considered valid
     - Linux/Unix:
       - Paths containing an ASCII null character are invalid
       - Strict mode also considers unprintable characters invalid (ASCII codes before space char)
     - Windows:
       - Unprintable characters are invalid (ASCII codes before space char), as well as reserved characters: `<>:"/|?*`
       - Colon `:` is valid after the drive letter (for absolute path), but otherwise invalid
       - A directory or file name _ending_ with a space or period is considered invalid, unless it's `"."` or `".."`
       - Strict mode also considers special reserved filenames as invalid (with or without an extension):
         - CON, PRN, AUX, NUL 
         - COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9
         - LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, LPT9
       - You may want to normalize_case() first to fix some things that would fail validation
     .
     \param  path    Path to validate
     \param  strict  Whether to enable additional checks for strict mode
     \return         Whether path is valid
    */
    static bool validate(const SubString& path, bool strict=true) {
        const char* p = path.data();
        const char* end = p + path.size();
        if (p == end)
            return false;

    #if defined(_WIN32)
        const char* CHARMAP = // invalid path chars: 0 for invalid, 1 for valid
            "0000000000000000000000000000000011011111110111101111111111010100"  // 00-63
            "1111111111111111111111111111111111111111111111111111111111110111"  // 64-127
            "1111111111111111111111111111111111111111111111111111111111111111"  // 128-191
            "1111111111111111111111111111111111111111111111111111111111111111"; // 191-255

        const char* name = NULL;
        if (path.size() >= 2 && p[1] == ':') {
            if (CHARMAP[(uchar)*p] == '0' || (path.size() > 2 && p[2] != PATH_DELIM))
                return false;
            p += 3;
            name = p;
        }

        for (; p < end; ++p) {
            if (*p == PATH_DELIM) {
                if (name != NULL && name < p && !valid_filename(name, (StrSizeT)(p - name), strict))
                    return false;
                name = p + 1;
            } else if (CHARMAP[(uchar)*p] == '0')
                return false;
        }
        if (name != NULL && name < p && !valid_filename(name, (StrSizeT)(p - name), strict))
            return false;

    #else
        if (strict) {
            for (; p < end; ++p)
                if ((uchar)*p < ' ')
                    return false;
        } else
            for (; p < end; ++p)
                if (*p == 0)
                    return false;
    #endif
        return true;
    }

    /** Validate whether filename is valid.
     - Filename is invalid if it contains any invalid characters
     - Non-ASCII UTF-8 characters are considered valid
     - This is different from a file path -- path delimiters are not valid in a filename
     - Linux/Unix:
       - Filenames containing an ASCII null character or slash are invalid
       - Strict mode also considers unprintable characters invalid (ASCII codes before space char)
     - Windows:
       - Unprintable characters are invalid (ASCII codes before space char), as well as reserved characters: `<>:"/\|?*`
       - A drive letter with a colon is not a valid filename
       - A filename _ending_ with a space or period is considered invalid, unless it's `"."` or `".."`
       - Strict mode also considers special reserved filenames as invalid (with or without an extension):
         - CON, PRN, AUX, NUL 
         - COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9
         - LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, LPT9
       - You may want to normalize_case() first to fix some things that would fail validation
     .
     \param  filename  Filename to validate
     \param  strict    Whether to enable additional checks for strict mode
     \return           Whether filename is valid
    */
    static bool validate_filename(const SubString& filename, bool strict=true) {
        const char* p = filename.data();
        const char* end = p + filename.size();
        if (p == end)
            return false;

    #if defined(_WIN32)
        const char* CHARMAP = // invalid filename chars: 0 for invalid, 1 for valid
            "0000000000000000000000000000000011011111110111101111111111010100"  // 00-63
            "1111111111111111111111111111011111111111111111111111111111110111"  // 64-127
            "1111111111111111111111111111111111111111111111111111111111111111"  // 128-191
            "1111111111111111111111111111111111111111111111111111111111111111"; // 191-255

        for (; p < end; ++p)
            if (CHARMAP[(uchar)*p] == '0')
                return false;
        if (!valid_filename(filename.data(), filename.size(), strict))
            return false;

    #else
        if (strict) {
            for (; p < end; ++p)
                if ((uchar)*p < ' ' || *p == PATH_DELIM)
                    return false;
        } else
            for (; p < end; ++p)
                if (*p == 0 || *p == PATH_DELIM)
                    return false;
    #endif
        return true;
    }

    /** Normalize path and remove redundant components.
     - This reads the input path, normallizes it, and returns an output path
     - This removes empty components (dup delims) and "." components and resolves ".." components
     - Normalizing a relative path keeps it from referencing parent directories and is useful for security reasons, though this may not be what you want
     .
     \param  outpath  Replaced with the resulting output path [out]
     \param  path     Input path to normalize -- ok if this is the object same as `outpath`
     \return          Reference to `outpath`
    */
    static String& normalize(String& outpath, const SubString& path) {
        const SubString CUR_DIR(".", 1);
        const SubString PARENT_DIR("..", 2);
        String output;
        output.setempty().reserve(path.size());

    #if defined(_WIN32)
        bool network_path = false;
        if (path.starts("\\\\", 2) || path.starts("//", 2)) {
            output.add(path.data(), 2);
            network_path = true;
        } else
    #endif
        if (path.starts(PATH_DELIM))
            output.add(PATH_DELIM); // preserve absolute path

        SubString input(path), value;
        while (input.token(value, PATH_DELIM)) {
            if (value == PARENT_DIR) {
                StrSizeT i = output.findr(PATH_DELIM);
            #if defined(_WIN32)
                if (i == END) {
                    if (network_path)
                        i = 2;  // truncate all for UNC, which requires a hostname
                    else if (output.size() >= 2 && output[1] == ':')
                        i = 3;  // truncate to drive path
                    else
                        i = 0;  // truncate relative path
                } else if (i == 1) {
                    if (output[0] == PATH_DELIM)
                        i = 0;  // truncate all for UNC, which requires a hostname
                } else if (i == 0) {
                    if (output.size() < 2 || output[1] != PATH_DELIM)
                        ++i; // keep non-UNC absolute path
                    // else truncate all for UNC, which requires a hostname
                } else if (i == 2 && output[1] == ':')
                    ++i;    // keep absolute path with drive
            #else
                if (i == END) {
                    i = 0; // truncate relative path
                } else if (i == 0)
                    ++i; // keep absolute path
            #endif
                output.truncate(i);
            } else if (value.size() > 0 && value != CUR_DIR) {
                if (output.size() > 0 && !output.ends(PATH_DELIM))
                    output.add(PATH_DELIM);
                output.add(value);
            }
        }

    #if defined(_WIN32)
        if (output.size() == 2 && output[1] == ':')
            output.add(PATH_DELIM);
    #endif

        outpath = output;
        return outpath;
    }

    /** Normalize path case.
     - Linux/Unix: This copies the input path as-is
     - Windows: This converts input path ASCII letters to lowercase, and also converts Unix-style slashes to backslashes
     .
     \param  outpath  Replaced with the resulting output path [out]
     \param  path     Input path to normalize case -- ok if this is the object same as `outpath`
     \return          Reference to `outpath`
    */
    static String& normalize_case(String& outpath, const SubString& path) {
        String output;
    #if defined(_WIN32)
        if (!path.null()) {
            output.setempty();
            const char* p = path.data();
            char* outp = output.advBuffer(path.size());
            for (const char* end = p + path.size(); p < end; ++p) {
                if (*p == PATH_DELIM_POSIX)
                    *outp = PATH_DELIM;
                else
                    *outp = ascii_tolower(*p);
                ++outp;
            }
        }
    #else
        output = path;
    #endif
        outpath = output;
        return outpath;
    }

    /** Get Windows drive from path.
     - Windows: This gets the drive or network share (if UNC) from `path`, if specified
       - This recognizes both slashes and backslashes as path delimiters, though Windows only supports backslashes -- see normalize_case()
       - If `path` contains a drive (drive letter and colon) then the drive letter and colon are included with the result
       - If `path` is a network share (UNC) path then host and share name (everything up to, but not including, the fourth backslash) are all considered the "drive"
         - Network share prefix can be slashes (`//`) or backslashes (`\\`), but not a mixture of both
       - Examples:
         - For `c:\foo` the drive is `c:`
         - For `c:foo` the drive is `c:`
         - For `\\host\share\foo` the drive is `\\host\share`
         - For `\foo` or `foo` the drive is null
     - Similar: split_drive()
     .
     \param  path  Path to extract drive from
     \return       Drive or network host (if UNC) from path, null if not specified in path -- always null outside of Windows
    */
    static SubString drive(const SubString& path) {
        SubString drv;
    #if defined(_WIN32)
        if (path.starts("\\\\", 2) || path.starts("//", 2)) {
            const SubString DELIMS("\\/", 2);
            StrSizeT i = path.findany(DELIMS.data(), DELIMS.size(), 2);
            if (i != NONE)
                i = path.findany(DELIMS.data(), DELIMS.size(), i + 1); // find end of share name
            if (i == NONE)
                drv = path;
            else
                drv.set(path, 0, i);
        } else if (path.size() >= 2 && path[1] == ':')
            drv.set(path, 0, 2);
    #endif
        return drv;
    }

    /** Get directory path from path.
     - The directory path is everything up to the the last PATH_DELIM occurrence, or null if no PATH_DELIM found
       - The last PATH_DELIM is only included in the result if it's also the first occurence in an absolute path
         - Linux/Unix example: `/`
         - Windows example: `C:\` or `\`
       - Windows: This recognizes both slashes and backslashes as path delimiters, though Windows itself only supports backslashes -- see normalize_case() to correct
     - Similar: split_dirpath()
     .
     \param  path  Path to extract directory path from
     \return       %Directory path from `path`, null
    */
    static SubString dirpath(const SubString& path) {
        SubString dir;
    #if defined(_WIN32)
        const SubString DELIMS("\\/:", 3);
        const StrSizeT i = path.findanyr(DELIMS.data(), DELIMS.size());
        if (i == 0) {
            dir.set(path, 0, 1);
        } else if (i == 1) {
            if (path[i] == ':')
                dir.set(path, 0, 2);
            else if (path[0] == path[1])
                dir.set(path, 0, 2);
            else
                dir.set(path, 0, i);
        } else if (i == 2 && path[1] == ':')
            dir.set(path, 0, 3);
    #else
        const StrSizeT i = path.findr(PATH_DELIM);
        if (i == 0)
            dir.set(path, 0, 1);
    #endif
        else if (i != NONE)
            dir.set(path, 0, i);
        return dir;
    }

    /** Get filename from path.
     - If `path` ends with a PATH_DELIM (or colon in Windows) then the returned filename will be empty
       - Windows: This recognizes both slashes and backslashes as path delimiters, though Windows itself only supports backslashes -- see normalize_case() to correct
     - Similar: split_filename()
     .
     \param  path  Path to extract filename from
     \return       Filename from `path`, same as `path` if no path delimiter found
    */
    static SubString filename(const SubString& path) {
        SubString name;
    #if defined(_WIN32)
        const SubString DELIMS("\\/:", 3);
        const StrSizeT i = path.findanyr(DELIMS.data(), DELIMS.size());
    #else
        const StrSizeT i = path.findr(PATH_DELIM);
    #endif
        if (i == NONE)
            name = path;
        else
            name.set(path, i + 1, ALL);
        return name;
    }

    /** Get base filename from path, which is the filename without the extension.
     - This effectively uses filename() to get the filename from `path`, then splits on the last '.' to get the base filename
       - If the filename starts with a '.' then this is considered part of the base filename (not used to split base/extension)
     - If `path` ends with a PATH_DELIM (or colon in Windows) then the filename is empty and the base filename will also be empty
       - Windows: This recognizes both slashes and backslashes as path delimiters, though Windows itself only supports backslashes -- see normalize_case() to correct
     - See also: filename_ext()
     .
     \param  path  Path or filename to extract base filename from
     \return       Base filename from `path`, null if path is null
    */
    static SubString filename_base(const SubString& path) {
        SubString name;
    #if defined(_WIN32)
        const SubString DELIMS_DOT(".\\/:", 4);
        const SubString DELIMS("\\/:", 3);
        const StrSizeT i = path.findanyr(DELIMS_DOT.data(), DELIMS_DOT.size());
        char ch;
        if (i == NONE || (i == 0 && path[i] == '.'))
            name = path;
        else if (path[i] != '.')
            name.set(path, i + 1, ALL);
        else if (i > 0 && ((ch=path[i-1]) == PATH_DELIM || ch == PATH_DELIM_POSIX || ch == ':'))
            name.set(path, i, ALL);
        else {
            const StrSizeT j = path.findanyr(DELIMS.data(), DELIMS.size(), 0, i);
            name.set2(path, (j == NONE ? 0 : j + 1), i);
        }
    #else
        const SubString DELIMS("./", 2);
        const StrSizeT i = path.findanyr(DELIMS.data(), DELIMS.size());
        if (i == NONE || (i == 0 && path[i] == '.'))
            name = path;
        else if (path[i] != '.')
            name.set(path, i + 1, ALL);
        else if (i > 0 && path[i-1] == PATH_DELIM)
            name.set(path, i, ALL);
        else {
            const StrSizeT j = path.findr(PATH_DELIM, 0, i);
            name.set2(path, (j == NONE ? 0 : j + 1), i);
        }
    #endif
        return name;
    }

    /** Get file extension from filename in path.
     - This effectively uses filename() to get the filename from `path`, then splits on the last '.' to get the file extension
     - If `path` ends with a PATH_DELIM (or colon in Windows) then the filename is empty and the extension will be null
       - Windows: This recognizes both slashes and backslashes as path delimiters, though Windows itself only supports backslashes -- see normalize_case() to correct
     - See also: filename_base()
     .
     \param  path  Path or filename to extract file extension from
     \return       %File extension from `path`, null if no extension found, empty if filename ends with a '.'
    */
    static SubString filename_ext(const SubString& path) {
    #if defined(_WIN32)
        const SubString DELIMS(".\\/:", 4);
    #else
        const SubString DELIMS("./", 2);
    #endif
        SubString name;
        const StrSizeT i = path.findanyr(DELIMS.data(), DELIMS.size());
        if (i == 0 && path[0] == '.')
            name.setempty();
        else if (i != NONE && path[i] == '.') {
        #if defined(_WIN32)
            char ch;
            if (i == 2 && path[1] == ':')
                name.setempty();
            else if (i > 0 && ((ch=path[i-1]) == PATH_DELIM || ch == PATH_DELIM_POSIX))
        #else
            if (i > 0 && path[i-1] == PATH_DELIM)
        #endif
                name.setempty();
            else
                name.set(path, i + 1, ALL);
        }
        return name;
    }

    /** Split path into list of components.
     - For an absolute path, the first component will be a path delimiter -- use abs() to check this
     - Windows: The first component will also include the drive or network share, if applicable, along with the path delimiter if an absolute path
       - Note that this is different from split_drive(), which does _not_ combine the drive and path delim for absolute paths
       - Use hasdrive() to check if first component has a drive or network share
       - Use abs() with `strict=false` to check if it's an absolute path
     - Reverse with: join_list()
     .
     \param  list  Stores path components (cleared first), set to null if path is null
     \param  path  Path to split
     \return       Reference to `list`
    */
    template<class T>
    static T& split_list(T& list, const SubString& path) {
        SubString remain;
        if (path.null()) {
            list.set();
        } else {
            list.setempty();
        #if defined(_WIN32)
            SubString a, b;
            a = split_drive(b, path);
            if (a.empty()) {
                if (path.starts(PATH_DELIM) || path.starts(PATH_DELIM_POSIX))
                    list.add(SubString(path, 0, 1));
            } else if (b.starts(PATH_DELIM) || b.starts(PATH_DELIM_POSIX)) {
                list.add(SubString(path, 0, a.size() + 1));
                b.triml(1);
            } else
                list.add(a);
            remain = b;
            const SubString DELIMS("\\/", 2);
            for (;;) {
                if (remain.splitat(remain.findany(DELIMS.data(), DELIMS.size()), a, b)) {
                    if (a.size() > 0)
                        list.add(a);
                    remain = b;
                } else {
                    if (a.size() > 0)
                        list.add(a);
                    break;
                }
            }
        #else
            SubString val;
            if (path.starts(PATH_DELIM)) {
                list.add(SubString("/", 1));
                remain.set(path, 1, ALL);
            } else
                remain = path;
            while (remain.token(val, PATH_DELIM))
                list.add(val);
        #endif
        }
        return list;
    }

    /** Get Windows drive and remaining path from path.
     - This works the same as drive(const SubString&) but also sets the remaining path in `outpath`
     - In all cases, concatenating drive and `outpath` will be the same as the input `path`
     - Reverse with: join_drive()
     - See also: drive()
     .
     \param  outpath  Stores remaining path after drive -- always same as `path` if not Windows [out]
     \param  path     Path to extract drive
     \return          Drive or network host (if UNC) from path, null if not specified in path -- always null if not Windows
    */
    static SubString split_drive(SubString& outpath, const SubString& path) {
        SubString drv;
    #if defined(_WIN32)
        if (path.starts("\\\\", 2) || path.starts("//", 2)) {
            const SubString DELIMS("\\/", 2);
            StrSizeT i = path.findany(DELIMS.data(), DELIMS.size(), 2);
            if (i != NONE)
                i = path.findany(DELIMS.data(), DELIMS.size(), i + 1); // find end of share name
            if (i == NONE) {
                drv = path;
                outpath.setempty();
            } else {
                drv.set(path, 0, i);
                outpath.set(path, i, ALL);
            }
        } else if (path.size() >= 2 && path[1] == ':') {
            drv.set(path, 0, 2);
            outpath.set(path, 2, ALL);
        } else 
            outpath = path;
    #else
        outpath = path;
    #endif
        return drv;
    }

    /** Get directory path from path.
     - The directory path is everything up to the the last PATH_DELIM occurrence, or null if no PATH_DELIM found
       - The last PATH_DELIM is only included in the result if it's also the first occurence in an absolute path
         - Linux/Unix example: `/`
         - Windows example: `C:\` or `\`
       - Windows: This recognizes both slashes and backslashes as path delimiters, though Windows itself only supports backslashes -- see normalize_case() to correct
     - Reverse with: join_dirpath()
     - See also: dirpath()
     .
     \param  outfilename  Stores filename from path, empty if no filename, null if path is null
     \param  path         Path to extract directory path from
     \return              %Directory path from `path`, null if `path` is null
    */
    static SubString split_dirpath(SubString& outfilename, const SubString& path) {
        SubString dir;
    #if defined(_WIN32)
        const SubString DELIMS("\\/:", 3);
        const StrSizeT i = path.findanyr(DELIMS.data(), DELIMS.size());
        if (i == 0) {
            dir.set(path, 0, 1);
            outfilename.set(path, 1, ALL);
        } else if (i == 1) {
            if (path[i] == ':') {
                dir.set(path, 0, 2);
                outfilename.set(path, 2, ALL);
            } else if (path[0] == path[1]) {
                dir.set(path, 0, 2);
                outfilename.set(path, 2, ALL);
            } else {
                dir.set(path, 0, i);
                outfilename.set(path, i + 1, ALL);
            }
        } else if (i == 2 && path[1] == ':') {
            dir.set(path, 0, 3);
            outfilename.set(path, i + 1, ALL);
        }
    #else
        const StrSizeT i = path.findr(PATH_DELIM);
        if (i == 0) {
            dir.set(path, 0, 1);
            outfilename.set(path, 1, ALL);
        }
    #endif
        else if (i != NONE) {
            dir.set(path, 0, i);
            outfilename.set(path, i + 1, ALL);
        } else if (!path.null())
            outfilename = path;
        return dir;
    }

    /** Split filename into base name and extension.
     - This assumes input `filename` is not a path, call filename() to get filename from path
     - This is more efficient than calling filename_base() and filename_ext() separately
     - If the filename starts with a '.' then this is considered part of the base name
     - Reverse with: join_filename()
     - See also: filename()
     .
     \param  ext       Stores file extension, set to null if no extension [out]
     \param  filename  Input filename to split
     \return           Base name, null if path is null
    */
    static SubString split_filename(SubString& ext, const SubString& filename) {
        SubString name;
        const StrSizeT i = filename.findr('.');
        if (i == NONE || i == 0) {
            name = filename;
            ext.set();
        } else {
            name.set(filename, 0, i);
            ext.set(filename, i + 1, ALL);
        }
        return name;
    }

    /** Split input path into drive, dirpath, and filename components.
     - This calls split_drive() and split_dirpath()
     .
     \param  drive     Stores drive from input `path`, without the rest of the path
     \param  dirpath   Stores directory path from input `path`, without drive or filename
     \param  filename  Stores filename from `path`, without directory path
     \param  path      Input path to split
    */
    static void split_all(SubString& drive, SubString& dirpath, SubString& filename, const SubString& path) {
        drive = FilePath::split_drive(dirpath, path);
        dirpath = FilePath::split_dirpath(filename, dirpath);
    }

    /** Split input path into drive, dirpath, file basename, and file extension components.
     - This calls split_drive(), split_dirpath(), and split_filename()
     .
     \param  drive     Stores drive from input `path`, without the rest of the path
     \param  dirpath   Stores directory path from input `path`, without drive or filename
     \param  basename  Stores file base name from filename in `path`, without file extension or path
     \param  ext       Stores file extension from filename in `path`, without base name or path
     \param  path      Input path to split
    */
    static void split_all(SubString& drive, SubString& dirpath, SubString& basename, SubString& ext, const SubString& path) {
        SubString filename;
        drive = FilePath::split_drive(dirpath, path);
        dirpath = FilePath::split_dirpath(filename, dirpath);
        basename = FilePath::split_filename(ext, filename);
    }

    /** Join two paths together.
     - This joins paths by appending `addpath` to `path`, but if `addpath` is an absolute path then it places the current `path`
     - Windows:
       - If `addpath` has a drive letter then it replaces `path`, even if not absolute
         - Example: `c:foo` is relative to current dir on drive `C:` while `c:\foo` is an absolute path -- either one replaces `path`
       - A network (UNC) path beginning with a double-backslash is considered an absolute path
       - A path beginning with a backslash is considered absolute, though it's relative to the current drive
     .
     \param  basepath  Starting path to join to
     \param  addpath   Additional path to join, appended to path if relative, replaces path if absolute (Windows: or if includes driver letter)
     \return           Reference to `path`
    */
    static String& join(String& basepath, const SubString& addpath) {
    #if defined(_WIN32)
        if (addpath.size() >= 2 && addpath[1] == ':')
            basepath = addpath;
        else
    #endif
        if (addpath.starts(PATH_DELIM)) {
            basepath = addpath;
        } else if (addpath.size() > 0) {
            basepath.addsep(PATH_DELIM);
            basepath.add(addpath);
        }
        return basepath;
    }

    /** Join list of path components.
     - Reverse with: split_list()
     .
     \param  out   Output string to write to (appended) [in/out]
     \param  list  %List of path components
     \return       Reference to `out`
    */
    template<class T>
    static String& join_list(String& out, const T& list) {
        const typename T::Size sz = list.size();
        if (sz > 0) {
            const String& first = list[0];
            out.add(first);

            typename T::Size i = 1;
            if (sz > 1 && (first.ends(PATH_DELIM_POSIX) || first.ends(':')))
                out.add(list[i++]);

            for (; i < sz; ++i) {
            #if defined(_WIN32)
                if (!out.ends(PATH_DELIM_POSIX))
            #endif
                out.addsep(PATH_DELIM);
                out.add(list[i]);
            }
        }
        return out;
    }

    /** Join drive and dirpath and write to output string.
     - Reverse with: split_drive()
     .
     \param  out      Output string to write to (appended) [in/out]
     \param  drive    Drive to join
     \param  dirpath  %Directory path to join
     \return          Reference to `out`
    */
    static String& join_drive(String& out, const SubString& drive, const SubString& dirpath) {
        return out.reserve(drive.size() + dirpath.size()).add(drive).add(dirpath);
    }

    /** Join dirpath and filename and write to output string.
     - Reverse with: split_dirpath()
     .
     \param  out      Output string to write to (appended) [in/out]
     \param  dirpath  %Directory path to join
     \param  filename Filename to join
     \return          Reference to `out`
    */
    static String& join_dirpath(String& out, const SubString& dirpath, const SubString& filename) {
        if (dirpath.empty())
            out.add(filename);
        else if (filename.empty())
            out.add(dirpath);
    #if defined(_WIN32)
        else if (dirpath.size() == 2 && dirpath[1] == ':') {
            out.reserve(dirpath.size() + filename.size());
            out.add(dirpath);
            out.add(filename);
        }
    #endif
        else {
            out.reserve(dirpath.size() + 1 + filename.size());
            out.add(dirpath);
        #if defined(_WIN32)
            if (!out.ends(PATH_DELIM_POSIX))
        #endif
            out.addsep(PATH_DELIM);
            out.add(filename);
        }
        return out;
    }

    /** Join file base name and extension components and write to output string.
     - Reverse with: split_filename()
     .
     \param  out       Output string to write to (appended) [in/out]
     \param  basename  %File base name to join
     \param  ext       %File extension to join
     \return           Reference to `out`
    */
    static String& join_filename(String& out, const SubString& basename, const SubString& ext) {
        if (basename.empty()) {
            if (ext.size() > 0)
                out.add('.').add(ext);
        } else if (ext.empty())
            out.add(basename);
        else {
            out.reserve(basename.size() + 1 + ext.size());
            out.add(basename);
            out.addsep('.');
            out.add(ext);
        }
        return out;
    }

    /** Join drive, dirpath, and filename components and write to output string.
     - Reverse with: split_all(SubString&,SubString&,SubString&,const SubString&)
     .
     \param  out       Output string to write to (appended) [in/out]
     \param  drive     Drive to join
     \param  dirpath   %Directory path to join
     \param  filename  Filename to join
     \return           Reference to `out`
    */
    static String& join_all(String& out, const SubString& drive, const SubString& dirpath, const SubString& filename) {
        if (drive.empty() && dirpath.empty())
            out.add(filename);
    #if defined(_WIN32)
        else if (dirpath.empty() && drive.ends(':')) {
            out.reserve(drive.size() + filename.size());
            out.add(drive);
            out.add(filename);
        }
    #endif
        else {
            out.reserve(drive.size() + dirpath.size() + 1 + filename.size());
            out.add(drive);
            out.add(dirpath);
        #if defined(_WIN32)
            if (!out.ends(PATH_DELIM_POSIX))
        #endif
            out.addsep(PATH_DELIM);
            out.add(filename);
        }
        return out;
    }

    /** Join drive, dirpath, and filename components and write to output string.
     - Reverse with: split_all(SubString&,SubString&,SubString&,SubString&,const SubString&)
     .
     \param  out       Output string to write to (appended) [in/out]
     \param  drive     Drive to join
     \param  dirpath   %Directory path to join
     \param  basename  %File base name to join
     \param  ext       %File extension to join
     \return           Reference to `out`
    */
    static String& join_all(String& out, const SubString& drive, const SubString& dirpath, const SubString& basename, const SubString& ext) {
        if (drive.empty() && dirpath.empty()) {
            out.reserve(basename.size() + 1 + ext.size());
            out.add(basename);
            if (ext.size() > 0) {
                out.addsep('.');
                out.add(ext);
            }
    #if defined(_WIN32)
        } else if (dirpath.empty() && drive.ends(':')) {
            out.reserve(drive.size() + basename.size() + 1 + ext.size());
            out.add(drive);
            out.add(basename);
            if (ext.size() > 0) {
                out.addsep('.');
                out.add(ext);
            }
    #endif
        } else {
            out.reserve(drive.size() + dirpath.size() + 2 + basename.size() + ext.size());
            out.add(drive);
            out.add(dirpath);
        #if defined(_WIN32)
            if (!out.ends(PATH_DELIM_POSIX))
        #endif
            out.addsep(PATH_DELIM);
            out.add(basename);
            if (ext.size() > 0) {
                out.addsep('.');
                out.add(ext);
            }
        }
        return out;
    }
};

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
