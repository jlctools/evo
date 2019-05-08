// Evo C++ Library
/* Copyright 2019 Justin Crowell
Distributed under the BSD 2-Clause License -- see included file LICENSE.txt for details.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file commandline.h Evo command-line parsing. */
#pragma once
#ifndef INCL_evo_config_h
#define INCL_evo_config_h

#include "strtok.h"
#include "string.h"
#include "maplist.h"
#include "setlist.h"
#include "ptr.h"
#include "io.h"

namespace evo {
/** \addtogroup EvoTools
Evo command-line and misc helper tools
*/
//@{

///////////////////////////////////////////////////////////////////////////////

/** Process command-line arguments.
 - Use `add*()` methods to add support for options/arguments/commands:
   - addflag() adds a flag option
   - addopt() adds an option with value, returns an Option object to further customize this option
   - addsep() adds an option separator in usage help, a blank line before the next option
   - addarg() adds an argument, returns an Option object to further customize this argument
   - addcmd() adds a sub-command, returns a Command object to further customize this command
 - This handles `-h` or `--help` flags automatically to show well formatted usage help and exit
   - If addver() is called, this also handles the `--version` flag automatically
   - Usage help shows options in the same order they're added
   - By default this will exit (terminate) on error or on help/version output, unless set_noexit() is called
 - Use parse() to parse and process a command-line and populate a command-line map
 - This uses Console by default for output, which is _not_ thread safe -- thread safety isn't normally needed here anyway
 .
 \tparam  ConsoleT  %Console type to use (used for testing, leave default)

\par Example

Here's a simple example:

\code
#include <evo/commandline.h>
using namespace evo;

const char* HELP = "Test program description for command-line argument parsing. Long lines will wrap automatically as needed.\n\
\nBullet list of things:\n\
 - First thing\n\
 - Second thing";

const char* EPILOG = "This is the epilog, which gives additional description and is shown last.";

int main(int argc, const char* argv[]) {
    Console& c = con();

    // Initialize with description and info
    CommandLine cmdline(HELP);
    cmdline.set_epilog(EPILOG);
    cmdline.addver("Program version 1.0");

    // Add options/arguments to support
    cmdline.addflag("-f, --flag", "Flag option");
    cmdline.addopt("-o, --opt", "Option with a value");
    cmdline.addarg("file", "File to use").required().multi();

    // Parse command-line, store results in args map
    StrMapList args;
    cmdline.parse(args, argc, argv);

    // Show results
    c.out << "Results:" << NL;
    for (StrMapList::Iter iter(args); iter; ++iter)
        c.out << ' ' << iter->key() << '=' << iter->value() << NL;
    c.out << NL;

    // Check if a flag is set using map contains()
    if (args.contains("flag"))
        c.out << "Flag is set" << NL;

    // Get option value using lookupsub()
    SubString value(lookupsub(args, "opt"));
    if (!value.null())
        c.out << "Opt: " << value << NL;

    // Get argument value using lookupsub(), tokenize multiple values on ';'
    StrTok tok(lookupsub(args, "file"));
    while (tok.next(';'))
        c.out << "File: " << tok.value() << NL;

    return 0;
}
\endcode

Output with: `./program -h`
\code{.unparsed}
Usage: program [options] file...

Test program description for command-line argument parsing. Long lines will wrap automatically as
needed.

Bullet list of things:
 - First thing
 - Second thing

Options:
 -f, --flag          Flag option
 -o, --opt <value>   Option with a value
 -h, --help          Show this usage help
 --help-general      Show general argument processing help
 --version           Show version information

Arguments:
 file   File to use

This is the epilog, which gives additional description and is shown last.
\endcode

Output with: `./program -f -o=val file1 file2`
\code{.unparsed}
Results:
 file=file1;file2
 flag=1
 opt=val

Flag is set
Opt: val
File: file1
File: file2
\endcode
*/
template<class ConsoleT=Console>
class CommandLineT {
public:
    typedef CommandLineT<ConsoleT> This;    ///< This type
    typedef typename ConsoleT::OutT OutT;   ///< Output stream type

    static const uint MAXLINE_DEFAULT = 100;    ///< Default maxline value

    /** Used to set additional option/argument information.
     - See addopt(), addarg(), add()
    */
    struct Option {
        /** Destructor. */
        virtual ~Option() { }

        /** %Set default value for option.
         - The default value is used when this option/argument is not given
         - The default is automatically appended to option/argument usage help
           - Usage help may contain `${default}` to control where the default value is shown, otherwise the default is appended to it
         .
         \return  This
        */
        virtual Option& default_value(const String& value) = 0;

        /** Make this option/argument required.
         - If a required option/argument is missing an error is given
         .
         \param  val  Whether required, false to make optional
         \return      This
        */
        virtual Option& required(bool val=true) = 0;

        /** Allow multiple values for option/argument.
         - For options, the values will accumulate when the option is repeated (instead of replace the previous value)
         - For arguments, the values will accumulate with each argument
         .
         \param  val  Whether to allow multiple values, false for single value
         \return      This
        */
        virtual Option& multi(bool val=true) = 0;

        /** Require option/argument value to be numeric.
         - Any value that isn't numeric (digits 0-9 only) will give an error
         .
         \return  This
        */
        virtual Option& numeric() = 0;

        /** %Set maximum value length, or max digits if required to be numeric.
         - Any value that exceeds this max length will give an error
         .
         \param  len  Maximum value length allowed, or max digits allowed if required to be numeric
         \return      This
        */
        virtual Option& maxlen(uint len) = 0;

        /** Add one or more preset choices.
         - This limits the option/argument value to one of the given choices -- any other value will give an error
         .
         \param  value  Choice values, separated by semi-colon (;)
         \return        This
        */
        virtual Option& addchoice(const SubString& value) = 0;

        /** Add one or more preset choices from string list.
         - This calls addchoice() for each string value in list
         .
         \tparam  T  %String list type (List, SubList, etc), inferred from argument
         \param  values  %List of string values to add as choices
         \return         This
        */
        template<class T>
        Option& addchoices(const T& values) {
            for (SizeT i = 0, sz = values.size(); i < sz; ++i)
                addchoice(values[i]);
            return *this;
        }
    };

    /** Used to set command options and arguments.
     - See addcmd()
    */
    struct Command {
        /** Destructor. */
        virtual ~Command() { }

        /** Parse input string and add an option or argument as described to command.
         - This calls addsep(), addflag(), addopt(), or addarg() depending on input
         - Use empty string to add an option separator
         - Option flag syntax: `NAMES  HELP`
         - Option with value syntax: `NAMES VALUE  HELP`
         - Argument syntax: `NAME  HELP`
         - Where:
           - `NAMES` is a comma and/or space separated list of short or long option names (usually short first), the last of which is used as the option `key` (without dash or double dash prefix)
           - `HELP` is a help string for this option/argument (may contain newlines) -- at least two spaces are required before this
           - `VALUE` is the name of the option value to show in usage help (also called `helpname`)
           - `NAME` is the argument name to show in usage help
         .
         \param  str  Input string to parse
        */
        virtual Option& add(const String& str) = 0;

        /** Add an options separator in usage help.
         - This adds a blank line before the next option in usage help
         - This is useful for visually grouping related options in usage help
         .
        */
        virtual void addsep() = 0;

        /** Add an option flag to command.
         - When parsing repeated flags, the occurrences are counted and stored as a string value (if more than 1 occurrence)
         - Special option '-' (just a single dash) can be added here, this usually means read from stdin instead of a file
         .
         \param  names  %Option names to look for when parsing arguments (comma separated)
         \param  key    Key to store option flag under when parsing arguments
         \param  help   Help string for this option
        */
        virtual void addflag(const String& names, const String& key, const String& help) = 0;

        /** Add an option flag to command.
         - The last name is used as the option `key` (without dash or double dash prefix)
         - When parsing repeated flags, the occurrences are counted and stored as a string value (if more than 1 occurrence)
         - Special option '-' (just a single dash) can be added here, this usually means read from stdin instead of a file
         .
         \param  names  %Option names to look for when parsing arguments (comma separated)
         \param  help   Help string for this option
        */
        virtual void addflag(const String& names, const String& help) = 0;

        /** Add an option that stores a value to command.
         - Multiple values allowed for the same option are stored with a semicolon separator
         - When parsing repeated options, previous values for that option are overwritten by default
         .
         \param  names     %Option names to look for when parsing arguments (comma separated), example: -v,--verbose
         \param  key       Key to store option flag under when parsing arguments, example: outfile
         \param  helpname  Name to represent option value shown with usage help (usually in caps), example: OUTFILE
         \param  help      Help string for this option to show with usage help, example: Output file to use
         \return           Option object for setting additonal information
        */
        virtual Option& addopt(const String& names, const String& key, const String& helpname, const String& help) = 0;

        /** Add an option that stores a value to command.
         - The last name is used as the option `key` (without dash or double dash prefix)
         - Multiple values allowed for the same option are stored with a semicolon separator
         - When parsing repeated options, previous values for that option are overwritten by default
         .
         \param  names     %Option names to look for when parsing arguments (comma separated), example: -v,--verbose
         \param  helpname  Name to represent option value shown with usage help (usually in caps), example: OUTFILE
         \param  help      Help string for this option to show with usage help, example: Output file to use
         \return           Option object for setting additonal information
        */
        virtual Option& addopt(const String& names, const String& helpname, const String& help) = 0;

        /** Add an option that stores a value to command.
         - The last name is used as both the option `key` and `helpname` (without dash or double dash prefix)
         - Multiple values allowed for the same option are stored with a semicolon separator
         - When parsing repeated options, previous values for that option are overwritten by default
         .
         \param  names     %Option names to look for when parsing arguments (comma separated), example: -v,--verbose
         \param  help      Help string for this option to show with usage help, example: Output file to use
         \return           Option object for setting additonal information
        */
        virtual Option& addopt(const String& names, const String& help) = 0;

        /** Add a positional argument to command.
         - Multiple values for an argument are stored with a semicolon separator (i.e. they stack)
         .
         \param  key       Key to store option flag under when parsing arguments, example: outfile
         \param  helpname  Name to represent argument value shown with usage help (usually in caps), example: OUTFILE
         \param  help      Help string for this option to show with usage help, example: Output file to use
         \return           Option object for setting additonal argument information
        */
        virtual Option& addarg(const String& key, const String& helpname, const String& help) = 0;

        /** Add a positional argument to command.
         - The key is also used as `helpname`
         - Multiple values for an argument are stored with a semicolon separator (i.e. they stack)
         .
         \param  key       Key to store option flag under when parsing arguments, example: outfile
         \param  help      Help string for this option to show with usage help, example: Output file to use
         \return           Option object for setting additonal argument information
        */
        virtual Option& addarg(const String& key, const String& help) = 0;
    };

private:
    enum Type {
        tPOS,       // positional argument
        tFLAG,      // flag option without a value
        tOPTION     // option with a single stored value
    };

    struct OptionInfo : public Option {
        Type   type;            // option/argument type
        String all;             // string with all option variants
        String key;             // key to store option under result map
        String helpname;        // option value name to show with help info
        String help;            // option description shown with help info
        String default_val;     // default option/argument value
        StrSetList choices;     // allowed choice values, ignored if empty
        ulong  bitflags;        // additional bit flags
        uint   max_length;      // maximum value length (or max digits if numeric)

        OptionInfo() : type(tPOS), bitflags(0), max_length(0) {
        }

        OptionInfo(Type type, const String& key, const String& help) :
            type(type),
            key(key),
            help(help),
            bitflags(0),
            max_length(0)
            { }

        OptionInfo(Type type, const String& key, const String& helpname, const String& help) :
            type(type),
            key(key),
            helpname(helpname),
            help(help),
            bitflags(0),
            max_length(0)
            { }

        OptionInfo(const OptionInfo& src) :
            type(src.type),
            all(src.all),
            key(src.key),
            helpname(src.helpname),
            help(src.help),
            default_val(src.default_val),
            choices(src.choices),
            bitflags(src.bitflags),
            max_length(0)
            { }

        Option& default_value(const String& value) {
            if (type == tOPTION)
                default_val = value;
            return *this;
        }

        Option& required(bool val=true) {
            if (val)
                bitflags |= BITFLAG_REQUIRED;
            else
                bitflags &= ~BITFLAG_REQUIRED;
            return *this;
        }

        Option& multi(bool val=true) {
            if (val)
                bitflags |= BITFLAG_MULTI_VALUE;
            else
                bitflags &= ~BITFLAG_MULTI_VALUE;
            return *this;
        }

        Option& numeric() {
            bitflags |= BITFLAG_NUMERIC;
            return *this;
        }

        Option& maxlen(uint len) {
            max_length = len;
            return *this;
        }

        Option& addchoice(const SubString& value) {
            SubString tok, tmpval(value);
            while (tmpval.token(tok, ';'))
                choices.add(tok);
            return *this;
        }

        String& format(String& str, bool shorthand=false) const {
            if (type == tPOS) {
                str.clear();
                const bool required = (bitflags & BITFLAG_REQUIRED);
                if (!required && !shorthand)
                    str << '[';
                if (!helpname.empty())
                    str << helpname;
                else
                    str << "VALUE";
                if (!shorthand) {
                    if (!required)
                        str << ']';
                    if (bitflags & BITFLAG_MULTI_VALUE)
                        str << "...";
                }
            } else {
                str = all;
                if (type != tFLAG) {
                    if (!helpname.empty())
                        str << ' ' << helpname;
                    else
                        str << " VALUE";
                }
            }
            return str;
        }

        template<class T>
        void store_flag(T& map) const {
            String& val = map[key];
            val.setn(*val.numu() + 1);
        }

        template<class T>
        void store_value(T& map, const SubString& value) const {
            if (bitflags & BITFLAG_MULTI_VALUE)
                map[key].addsep(';').add(value);
            else
                map[key] = value;
        }

    private:
        OptionInfo& operator=(const OptionInfo& src);
    };

    typedef SharedPtr<OptionInfo> OptionInfoShPtr;
    typedef MapList<String, OptionInfoShPtr> OptionMap;
    typedef List<OptionInfoShPtr> OptionList;
    typedef List<OptionInfo> ArgList;

    struct NullOption : public Option {
        Option& default_value(const String&)  { return *this; }
        Option& required(bool val=true)       { (void)val; return *this; }
        Option& multi(bool val=true)          { (void)val; return *this; }
        Option& numeric()                     { return *this; }
        Option& maxlen(uint)                  { return *this; }
        Option& addchoice(const SubString&)   { return *this; }
    };

    struct CommandInfo : public Command {
        typedef CommandLineT<ConsoleT> Parent;

        Parent*    parent;
        String     name;
        String     helptext;
        const OptionMap* parent_options; // pointer to global options when in subcommand, otherwise null
        OptionMap  options;
        OptionList options_list;
        ArgList    args;

        CommandInfo() : parent(NULL), parent_options(NULL) {
        }

        Option& add(const String& str) {
            static NullOption null;
            if (str.empty()) {
                addsep();
                return null;
            }

            SubString lines(str), line;
            if (lines.token_line(line)) {
                StrTok tok(line);
                if (tok.nextw(' ')) {
                    StrSizeT i;
                    if (tok.value().starts('-')) {
                        // Flag/Option
                        String names(tok.value()), help;
                        SubString value;
                        for (;;) {
                            i = tok.skipws();
                            if (!tok.nextw(' '))
                                break;

                            if (tok.value().starts('-')) {
                                names.addsep(',').add(tok.value());
                            } else {
                                if (i > 2 && (line[i-1] != ' ' || line[i-2] != ' ')) {
                                    i = tok.skipws();
                                    value = tok.value();
                                }
                                break;
                            }
                        }
                        get_help_text(help, str, i);
                        if (value.empty()) {
                            addflag(names, help);
                            return null;
                        } else
                            return addopt(names, value, help);
                    } else {
                        // Argument
                        SubString key(tok.value());
                        i = tok.skipws();
                        if (tok.nextw(' ') && tok.index() != NONE) {
                            String help;
                            get_help_text(help, str, i);
                            return addarg(key, help);
                        } else
                            return addarg(key, String());
                    }
                }
            }

            parent->show_warning() << "CommandLine::add() ignoring malformed input: " << line << parent->newline_;
            return null;
        }

        void addsep() {
            OptionInfoShPtr sep;
            options_list.add(sep);
        }

        void addflag(const String& names, const String& key, const String& help) {
            OptionInfoShPtr arg( new OptionInfo(tFLAG, key, help) );
            String all;
            StrTok tok(names);
            while (tok.next(',')) {
                options[SubString(tok.value()).stripl('-')] = arg;
                all.addsep(',').addsep(' ') << tok.value();
            }
            arg->all = all;
            options_list.add(arg);
        }

        void addflag(const String& names, const String& help) {
            SubString key;
            get_option_key(key, names);
            if (key.empty())
                parent->show_warning() << "CommandLine::addflag() ignoring malformed names: " << names << parent->newline_;
            else
                addflag(names, key, help);
        }

        Option& addopt(const String& names, const String& key, const String& helpname, const String& help) {
            OptionInfoShPtr arg( new OptionInfo(tOPTION, key, helpname, help) );
            String all;
            StrTok tok(names);
            while (tok.next(',')) {
                options[SubString(tok.value()).stripl('-')] = arg;
                all.addsep(',').addsep(' ') << tok.value();
            }
            arg->all = all;
            options_list.add(arg);
            return *arg;
        }

        Option& addopt(const String& names, const String& helpname, const String& help) {
            SubString key;
            get_option_key(key, names);
            if (key.empty()) {
                parent->show_warning() << "CommandLine::addopt() ignoring malformed names: " << names << parent->newline_;
                static NullOption null;
                return null;
            } else
                return addopt(names, key, helpname, help);
        }

        Option& addopt(const String& names, const String& help) {
            return addopt(names, "<value>", help);
        }

        Option& addarg(const String& key, const String& helpname, const String& help) {
            OptionInfo& arg = args(args.addnew().iend());
            arg.key      = key;
            arg.helpname = helpname;
            arg.help     = help;
            return arg;
        }

        Option& addarg(const String& key, const String& help) {
            return addarg(key, key, help);
        }

        // Scan options list and remove outdated items (dups)
        void option_cleanup() {
            SubString name_sub;
            for (SizeT j = options_list.size(); j > 0; ) {
                const OptionInfo* ptr = options_list[--j].ptr();
                if (ptr != NULL) {
                    ptr->all.split(',', name_sub);
                    const OptionInfoShPtr* findptr = options.find(name_sub.stripl('-'));
                    if (findptr == NULL || findptr->ptr() != ptr)
                        options_list.remove(j);
                }
            }
        }

        // Set defaults
        template <class T>
        void set_defaults(T& map) {
            for (typename OptionList::Iter iter(options_list); iter; ++iter) {
                const OptionInfo* ptr = iter->ptr();
                if (ptr != NULL && !ptr->default_val.null())
                    map[ptr->key] = ptr->default_val;
            }
        }

    private:
        void get_option_key(SubString& key, const SubString& names) {
            StrTok tok(names);
            while (tok.next(',')) {
                SubString tokname(tok.value());
                if (tokname == "-")
                    key = tokname;
                else if (tokname.starts("--") || (key.empty() && tokname.starts("-")))
                    key = tokname.stripl('-');
            }
        }

        void get_help_text(String& help, const SubString& str, StrSizeT indent=0) {
            help.set();
            SubString lines(str), line;
            if (indent == NONE)
                lines.token_line(line);
            for (uint i = 0; lines.token_line(line); ) {
                if (indent == NONE) {
                    indent = line.findanybut(" ", 1);                    
                    if (indent == NONE)
                        indent = 0;
                }
                if (indent > 0) {
                    if (i > 0) {
                        const StrSizeT maxtrim = line.findanybut(" ", 1);
                        line.triml(indent <= maxtrim ? indent : maxtrim);
                    } else {
                        line.triml(indent);
                        ++i;
                    }
                }
                if (!line.stripr().empty())
                    help.addsep('\n').add(line);
            }
        }
    };

    typedef MapList<String,CommandInfo> CommandList;

public:
    /** Default constructor.
     - Usage help will not include a description
     .
    */
    CommandLineT() : con_(ConsoleT::get()), progname_set_(false), maxline_(This::MAXLINE_DEFAULT), noexit_(false), error_(false) {
        main_.parent = this;
    }

    /** Constructor with help description.
     - The description is shown with usage help
     .
     \param  description  Desription to show with usage help
    */
    CommandLineT(const String& description) : con_(ConsoleT::get()), progname_set_(false), description_(description), maxline_(This::MAXLINE_DEFAULT), noexit_(false), error_(false) {
        main_.parent = this;
    }

    /** Get reference to console object used.
     \return  %Console reference
    */
    ConsoleT& get_con() {
        return con_;
    }

    /** Get current newline value used for output.
     \return  Newline value used
    */
    const NewlineValue& get_newline() const {
        return newline_;
    }

    /** %Set current newline value used for output.
     \param  nl  Newline value to use
     \return     This
    */
    This& set_newline(const NewlineValue& nl) {
        newline_ = nl;
        return *this;
    }

    /** %Set new max line length for usage help output.
     \param  maxline  New max line length, set to 40 if lower than 40
     \return          This
    */
    This& set_maxline(uint maxline) {
        const uint MIN_MAXLINE = 40;
        if (maxline < MIN_MAXLINE)
            maxline = MIN_MAXLINE;
        maxline_ = maxline;
        return *this;
    }

    /** %Set program name to use with usage help.
     - If not set, then this is inferred from the first argument parsed
     - If set, you may need to pass `offset=1` to parse to skip argv program name
     .
     \param  name  Program name to use, ignored if empty
     \return       This
    */
    This& set_progname(const String& name) {
        progname_ = name;
        progname_set_ = true;
        return *this;
    }

    /** %Set epilog text shown in usage help.
     - This should include things like relevant environment variables and the software homepage
     .
     \param  text  Epilog text
     \return       This
    */
    This& set_epilog(const String& text) {
        epilog_ = text;
        return *this;
    }

    /** %Set no-exit flag to prevent parse() from terminating and instead have it return false.
     \param  val  No-exit flag to set, false to let parse() terminate
     \return      This
    */
    This& set_noexit(bool val=true) {
        noexit_ = val;
        return *this;
    }

    /** Add version info and enable option flags to show it.
     - Version info is shown if `--version` or `-V` option is supplied on the command-line
     - See [GNU Standard for version info](https://www.gnu.org/prep/standards/html_node/_002d_002dversion.html#g_t_002d_002dversion)
     .
     \param  version_info  Version info string to show with version flags (--version or -V), empty to disable these flags
     \return               This
    */
    This& addver(const String& version_info) {
        version_ = version_info;
        return *this;
    }

    /** Parse input string and add an global option or argument as described.
     - This calls addsep(), addflag(), addopt(), or addarg() depending on input
     - Use empty string to add an option separator
     - Option flag syntax: `NAMES  HELP`
     - Option with value syntax: `NAMES VALUE  HELP`
     - Argument syntax: `NAME  HELP`
     - Where:
       - `NAMES` is a comma and/or space separated list of short or long option names (usually short first), the last of which is used as the option `key` (without dash or double dash prefix)
       - `HELP` is a help string for this option/argument (may contain newlines) -- at least two spaces are required before this
       - `VALUE` is the name of the option value to show in usage help (also called `helpname`)
       - `NAME` is the argument name to show in usage help
     .
     \param  str  Input string to parse
    */
    Option& add(const String& str) {
        return main_.add(str);
    }

    /** Add a global options separator in usage help.
     - This adds a blank line before the next option in usage help
     - This is useful for visually grouping related options in usage help
     .
    */
    void addsep() {
        main_.addsep();
    }

    /** Add a global option flag.
     - When parsing repeated flags, the occurrences are counted and stored as a string value (if more than 1 occurrence)
     - Special option '-' (just a single dash) can be added here, this usually means read from stdin instead of a file
     .
     \param  names  %Option names to look for when parsing arguments (comma separated)
     \param  key    Key to store option flag under when parsing arguments
     \param  help   Help string for this option
    */
    void addflag(const String& names, const String& key, const String& help) {
        return main_.addflag(names, key, help);
    }

    /** Add a global option flag.
     - This calls addflag(const String&,const String&,const String&) with `key` from last option in `names`
     .
     \param  names  %Option names to look for when parsing arguments (comma separated)
     \param  help   Help string for this option
    */
    void addflag(const String& names, const String& help) {
        return main_.addflag(names, help);
    }

    /** Add a global option that stores a value.
     - Multiple values allowed for the same option are stored with a semicolon separator
     - When parsing repeated options, previous values for that option are overwritten by default
     .
     \param  names     %Option names to look for when parsing arguments (comma separated), example: -v,--verbose
     \param  key       Key to store option flag under when parsing arguments, example: outfile
     \param  helpname  Name to represent option value shown with usage help (usually in caps), example: OUTFILE
     \param  help      Help string for this option to show with usage help, example: Output file to use
     \return           Option object for setting additonal information
    */
    Option& addopt(const String& names, const String& key, const String& helpname, const String& help) {
        return main_.addopt(names, key, helpname, help);
    }

    /** Add a global option that stores a value.
     - This effectively calls addopt(const String&,const String&,const String&,const String&) with `key` derived from last option in `names`
     .
     \param  names     %Option names to look for when parsing arguments (comma separated), example: -v,--verbose
     \param  helpname  Name to represent option value shown with usage help (usually in caps), example: OUTFILE
     \param  help      Help string for this option to show with usage help, example: Output file to use
     \return           Option object for setting additonal information
    */
    Option& addopt(const String& names, const String& helpname, const String& help) {
        return main_.addopt(names, helpname, help);
    }

    /** Add a global option that stores a value.
     - This effectively calls addopt(const String&,const String&,const String&,const String&) with:
       - `key` derived from last option in `names`
       - and `helpname` defaulting to: `<value>`
     .
     \param  names  %Option names to look for when parsing arguments (comma separated), example: -v,--verbose
     \param  help   Help string for this option to show with usage help, example: Output file to use
     \return        Option object for setting additonal information
    */
    Option& addopt(const String& names, const String& help) {
        return main_.addopt(names, help);
    }

    /** Add a positional argument.
     - Multiple values for an argument are stored with a semicolon separator (i.e. they stack)
     .
     \param  key       Key to store option flag under when parsing arguments, example: outfile
     \param  helpname  Name to represent argument value shown with usage help (usually in caps), example: OUTFILE
     \param  help      Help string for this option to show with usage help, example: Output file to use
     \return           Option object for setting additonal information
    */
    Option& addarg(const String& key, const String& helpname, const String& help) {
        return main_.addarg(key, helpname, help);
    }

    /** Add a positional argument.
     - This effectively calls addarg(const String&,const String&,const String&) with `helpname` copied from `key`
     .
     \param  key   Key to store option flag under when parsing arguments, example: outfile
     \param  help  Help string for this option to show with usage help, example: Output file to use
     \return       Option object for setting additonal information
    */
    Option& addarg(const String& key, const String& help) {
        return main_.addarg(key, help);
    }

    /** Add a sub-command argument with it's own options and arguments.
     - Use the returned object to add options and arguments to this sub-command
     .
     \param  name  %Command name to add
     \param  help  Short help for command, shown in main usage help
     \return       Command object for setting additional information
    */
    CommandInfo& addcmd(const String& name, const String& help) {
        CommandInfo& cmd = subcommands_[name];
        if (cmd.parent == NULL) {
            cmd.parent = this;
            cmd.name = name;
            cmd.helptext = help;
            subcommand_list_.add(name);
        }
        return cmd;
    }

    /** Check whether an error occurred while parsing arguments.
     - This is true when something is wrong with a command-line argument
     .
     \return  Whether an error occurrend
    */
    bool error() const {
        return error_;
    }

    /** Parse and process command-line using current option and argument info.
     - This writes error or help info to stderr (using Console)
     - This writes the parsed result values to map
     - If this returns false, call error() to determine whether to terminate with an error
     .
     \param  map     %Map to write parsed options and arguments to
     \param  argc    Argument count (from `main()`)
     \param  argv    Argument array (from `main()`)
     \param  offset  Offset for starting argument (0 for first, 1 for second, etc)
     \return         Whether to continue, false to terminate due to an error or for showing help info
    */
    template<class TMap>
    bool parse(TMap& map, int argc, const char* argv[], int offset=0) {
        int i = (offset >= 0 ? offset : 0);
        if (progname_.null() || !progname_set_) {
            if (i < argc) {
                // Use first arg for program name
                Char delim;
                SubString name;
                SubString(argv[i]).tokenr_any(name, delim, "/\\", 2);
                progname_ = name;
                ++i;
            }
        }
        if (progname_.empty()) {
            con_.err << "ERROR: CommandLine processing not setup correctly -- No program name set" << newline_;
            return finish(true);
        }

        // Add support for builtin options
        addflag("-h, --help", "evo_help_", (subcommands_.size() > 0 ? "Show this usage help, or command usage help if after command" : "Show this usage help"));
        addflag("--help-general", "evo_help_general_", "Show general argument processing help");
        if (!version_.empty())
            addflag("--version", "evo_version_", "Show version information");
        cleanup();

        // Scan for builtin options first
        {
            const CommandInfo* cmd = NULL;
            for (int j = i; j < argc; ++j) {
                SubString opt(argv[j]);
                if (opt.starts("-")) {
                    if (opt == "--") {
                        break;
                    } else if (opt == "-h" || opt == "--help") {
                        print_help(cmd);
                        return finish(false);
                    } else if (opt == "--help-general") {
                        print_help_general();
                        return finish(false);
                    } else if (opt == "--version") {
                        if (!version_.empty()) {
                            con_.out << FmtStringWrap(version_, maxline_).set_newline(newline_);
                            return finish(false);
                        }
                    }
                } else {
                    const CommandInfo* new_cmd = subcommands_.find(opt);
                    if (new_cmd != NULL)
                        cmd = new_cmd;
                }
            }
        }

        // Parse arguments
        main_.set_defaults(map);
        ParseState state(main_);
        for (; i < argc; ++i) {
            if (!parse_arg(state, map, argv[i]))
                return false;
        }

        // Check for missing arguments
        if (state.current != NULL && state.valnum > 0)
            ++state.argnum; // current has at least 1 value
        while (state.argnum < state.cur_args->size()) {
            if ((*state.cur_args)[state.argnum].bitflags & BITFLAG_REQUIRED) {
                con_.err << progname_ << ": ERROR: Missing required argument: " << (*state.cur_args)[state.argnum].helpname << newline_;
                return finish(true);
            }
            ++state.argnum;
        }
        if (subcommands_.size() > 0 && state.cur_cmd == NULL) {
            con_.err << progname_ << ": ERROR: Missing required command" << newline_;
            return finish(true);
        }
        return true;
    }

    /** This writes the beginning (prefix) of an error message and returns a stream for the caller to write the error message to.
     - This sets up the error using formatting consistent with command-line parsing errors
     - Example:\code
       CommandLine cmdline;
       cmdline.show_error() << "An error occurred" << NL;
       exit(1);
       \endcode
     .
     \return  %Stream to write the desired error message, which should end with a newline (NL)
    */
    OutT& show_error() {
        if (!progname_.empty())
            con_.err << progname_ << ": ";
        con_.err << "ERROR: ";
        return con_.err;
    }

    /** This writes the beginning (prefix) of a warning message and returns a stream for the caller to write the warning message to.
     - This sets up the warning using formatting consistent with command-line parsing errors
     - Example:\code
       CommandLine cmdline;
       cmdline.show_warning() << "This is a warning message" << NL;
       \endcode
     .
     \return  %Stream to write the desired warning message, which should end with a newline (NL)
    */
    OutT& show_warning() {
        if (!progname_.empty())
            con_.err << progname_ << ": ";
        con_.err << "WARNING: ";
        return con_.err;
    }

private:
    static const ulong BITFLAG_REQUIRED     = 0x01;     // value required (non-flags)
    static const ulong BITFLAG_MULTI_VALUE  = 0x02;     // multiple values allowed (non-flags)
    static const ulong BITFLAG_DEFAULT_TRUE = 0x04;     // option flag defaults to true (flags)
    static const ulong BITFLAG_NUMERIC      = 0x08;     // validate numeric value

    ConsoleT& con_;
    NewlineValue newline_;
    String progname_;
    bool   progname_set_;
    String description_;
    String epilog_;
    String version_;
    uint maxline_;
    bool noexit_;
    bool error_;

    CommandInfo main_;
    CommandList subcommands_;
    List<String> subcommand_list_;

    // Called to finish parsing and exit if applicable
    bool finish(bool err) {
        if (!noexit_)
            ::exit(1); // COV can't test
        error_ = err;
        return false;
    }

    // Cleanup invalid/dup options
    void cleanup() {
        main_.option_cleanup();
    }

    // State while parsing
    struct ParseState {
        const OptionInfo*  current;     // current option/argument, set when expecting at least another value
        const OptionInfo*  prev_arg;    // previous argument with multiple values
        const CommandInfo* cur_cmd;     // current command, NULL if none
        const OptionMap*   cur_opts;    // current options, either &main_.options or command-specific while in subcommand
        const ArgList*     cur_args;    // current command arguments, either &main_.args or command-specific while in subcommand
        uint argnum;                    // current argument index
        uint valnum;                    // current value index, 0 if no value yet, 1 or more if at at least 1 value found
        bool end_options;               // true to stop detecting options

        ParseState(const CommandInfo& cmd) : current(NULL), prev_arg(NULL), cur_cmd(NULL), cur_opts(&cmd.options), cur_args(&cmd.args), argnum(0), valnum(0), end_options(false) {
        }

        void value_expected(const OptionInfo& info) {
            current = &info;
            valnum = 0;
        }

        void value_stored(const OptionInfo& info) {
            if (info.type != tPOS) {
                if (prev_arg != NULL) {
                    current = prev_arg; // return to multi-value argument before option
                    valnum = 1;
                } else {
                    current = NULL; // single value only
                    valnum = 0;
                }
            } else if (info.bitflags & BITFLAG_MULTI_VALUE) {
                current = &info; // allow additional values
                valnum = 1;
                prev_arg = current;
            }
        }

        void value_added(const OptionInfo& info) {
            if (info.type == tPOS) {
                if (info.bitflags & BITFLAG_MULTI_VALUE) {
                    ++valnum;
                    return;
                }
            } else if (prev_arg != NULL) {
                current = prev_arg; // return to multi-value argument before option
                valnum = 1;
            }            
            current = NULL; // single value only
            valnum = 0;
        }
    };

    // Lookup option, try current command options first, fallback to global options
    const OptionInfo* option_lookup(const ParseState& state, const SubString& name) const {
        const OptionInfoShPtr* result;
        if (state.cur_cmd != NULL) {
            result = state.cur_cmd->options.find(name);
            if (result != NULL)
                return result->ptr();
        }
        result = main_.options.find(name);
        if (result != NULL)
            return result->ptr();
        return NULL;
    }

    bool parse_validate(const OptionInfo& info, const SubString& value) {
        if (info.bitflags & BITFLAG_NUMERIC) {
            StrSizeT i = 0, sz = value.size();
            if (i < sz && value[i] == '-')
                ++i;
            for (StrSizeT digits = 0; i < sz; ++i, ++digits) {
                if (value[i] < '0' || value[i] > '9') {
                    con_.err << progname_ << ": ERROR: Value must be numeric: " << value << newline_;
                    return finish(true);
                }
                if (info.max_length > 0 && digits >= info.max_length) {
                    con_.err << progname_ << ": ERROR: Numeric value too long (max digits: " << info.max_length << "): " << value << newline_;
                    return finish(true);
                }
            }
        } else if (info.max_length > 0 && value.size() > info.max_length) {
            con_.err << progname_ << ": ERROR: Value too long (max length: " << info.max_length << "): " << value << newline_;
            return finish(true);
        }
        if (info.choices.size() > 0 && !info.choices.contains(value)) {
            con_.err << progname_ << ": ERROR: Invalid value: " << value << newline_;
            return finish(true);
        }
        return true;
    }

    template<class TMap>
    bool parse_arg(ParseState& state, TMap& map, const SubString& arg) {
        if (!state.end_options) {
            if (arg == "-.") {
                // Stop current multi-value argument
                state.current = NULL;
                state.prev_arg = NULL;
                return true;
            } else if (arg == "--") {
                // Stop processing options
                state.end_options = true;
                state.current = NULL;
                return true;
            }
        }

        if (state.current == NULL && state.prev_arg != NULL) {
            // Switch back to previous argument with multiple values
            state.current = state.prev_arg;
            state.valnum  = 1;
        }

        if (state.current != NULL) {
            if (!state.end_options && arg.starts('-') && state.valnum > 0) {
                // Found an option between arguments
                if (state.current->type == tPOS && state.current->bitflags & BITFLAG_MULTI_VALUE)
                    state.prev_arg = state.current; // save multi-value argument to come back to
                state.current = NULL;
                state.valnum = 0;
            } else {
                // Value for current option/argument
                const OptionInfo& info = *state.current;
                if (!parse_validate(info, arg))
                    return false;
                info.store_value(map, arg);
                state.value_added(info);
                return true;
            }
        }

        if (!state.end_options) {
            // Check for options
            SubString str, name, value;
            if (arg.starts("--", 2)) {
                // Long option
                str.set(arg, 2);
                const bool found_value = str.split('=', name, value);

                const OptionInfo* infop = option_lookup(state, name);
                if (infop == NULL) {
                    if (name.starts("no-", 3)) {
                        // Check for --no-* option to reset (remove) option
                        SubString noname(name, 3);
                        infop = option_lookup(state, noname);
                        if (infop != NULL) {
                            if (found_value) {
                                con_.err << progname_ << ": ERROR: Value not allowed with reset option: --" << name << newline_;
                                return finish(true);
                            }
                            map.remove(infop->key);
                            return true;
                        }
                    }
                    con_.err << progname_ << ": ERROR: Unknown option: --" << name << newline_;
                    return finish(true);
                }

                const OptionInfo& info = *infop;
                if (info.type == tFLAG) {
                    if (found_value) {
                        con_.err << progname_ << ": ERROR: Unexpected value with option: --" << name << newline_;
                        return finish(true);
                    }
                    info.store_flag(map);
                } else if (found_value) {
                    if (!parse_validate(info, value))
                        return false;
                    info.store_value(map, value);
                    state.value_stored(info);
                } else
                    state.value_expected(info);
                return true;
            } else if (arg.starts('-')) {
                // Short options
                const SubString args(arg.data() + 1, arg.size() - 1);
                for (StrSizeT i = 0, c = args.size(); i < c; ++i) {
                    if (args[i] == 'h') {
                        print_help(state.cur_cmd);
                        return finish(false);
                    } else {
                        str.set(args.data() + i, 1);
                        const OptionInfo* infop = option_lookup(state, str);
                        if (infop == NULL) {
                            con_.err << progname_ << ": ERROR: Unknown option: -" << str << newline_;
                            return finish(true);
                        }

                        const OptionInfo& info = *infop;
                        if (info.type == tFLAG) {
                            if (i + 1 < c && args[i + 1] == '=') {
                                con_.err << progname_ << ": ERROR: Unexpected value with option: -" << args[i] << newline_;
                                return finish(true);
                            }
                            info.store_flag(map);
                        } else {
                            if (i + 1 >= c) {
                                state.value_expected(info);
                                return true; // Value expected in next argument
                            }
                            if (args[i + 1] == '=') {
                                ++i;
                            } else if (i > 0) {
                                str.set(args.data() + i + 1, c - i - 1);
                                con_.err << progname_ << ": ERROR: Possible typo, use '" << args[i] << '=' << str << "' for clarity when combining short options in: -" << args << newline_;
                                return finish(true);
                            }

                            str.set(args.data() + i + 1, c - i - 1);
                            if (!parse_validate(info, str))
                                return false;
                            info.store_value(map, str);
                            state.value_stored(info);
                            break;
                        }
                    }
                }
                return true;
            }
        }

        // Argument value
        if (state.argnum >= state.cur_args->size()) {
            if (subcommands_.size() > 0 && state.cur_cmd == NULL) {
                // Sub-command argument
                state.cur_cmd = subcommands_.find(arg);
                if (state.cur_cmd == NULL) {
                    con_.err << progname_ << ": ERROR: Unknown command: " << arg << newline_;
                    return finish(true);
                }
                map["command"] = arg;
                state.current  = NULL;
                state.prev_arg = NULL;
                state.cur_args = &(state.cur_cmd->args);
                state.argnum   = 0;
                state.valnum   = 0;
                return true;
            }

            con_.err << progname_ << ": ERROR: Unexpected argument: " << arg << newline_;
            return finish(true);
        }

        const OptionInfo& info = (*state.cur_args)[state.argnum];
        if (!parse_validate(info, arg))
            return false;
        info.store_value(map, arg);
        state.value_stored(info);
        ++state.argnum;
        return true;
    }

    void print_help_usage(const CommandInfo& cmd, const char* options_prefix="") const {
        if (cmd.options.size() > 0)
            con_.out << " [" << options_prefix << "options]";
        String tmp;
        for (typename ArgList::Iter iter(cmd.args); iter; ++iter) {
            const OptionInfo& info = *iter;
            con_.out << ' ' << info.format(tmp);
        }
    }

    void print_help_args(const CommandInfo& cmd, const char* options_prefix="") const {
        const SubString SEP("   ", 3);
        String tmp;
        if (cmd.options.size() > 0) {
            con_.out << newline_ << options_prefix << "Options:" << newline_;

            const StrSizeT MAX_MAXLEN = 30;
            StrSizeT maxlen = 0;
            for (typename OptionList::Iter iter(cmd.options_list); iter; ++iter) {
                const OptionInfoShPtr& info = *iter;
                if (info) {
                    StrSizeT len = info->format(tmp).size();
                    if (len > maxlen)
                        maxlen = len;
                }
            }
            if (maxlen > MAX_MAXLEN)
                maxlen = MAX_MAXLEN;

            String help, help_default;
            const StrSizeT help_indent = maxlen + SEP.size() + 1;
            for (typename OptionList::Iter iter(cmd.options_list); iter; ++iter) {
                const OptionInfoShPtr& info = *iter;
                if (!info) {
                    con_.out << newline_;
                    continue;
                }

                con_.out << ' ' << FmtString(info->format(tmp), maxlen);
                if (tmp.size() > maxlen)
                    con_.out << newline_ << FmtChar(' ', maxlen + 1);

                if (info->help.empty()) {
                    con_.out << newline_;
                } else {
                    help = info->help;
                    if (!info->default_val.null()) {
                        // Add default to help, replace ${default} token, or append it
                        help_default.set() << "[default: " << info->default_val << ']';
                        if (help.findreplace("${default}", 10, help_default) == 0)
                            help.addsep(' ') << help_default;
                    }
                    con_.out << SEP << FmtStringWrap(help, maxline_ - help_indent).set_indent(help_indent).set_newline(newline_);
                }
            }
        }

        if (cmd.args.size() > 0) {
            bool found_help = false;
            StrSizeT maxlen = 0;
            for (typename ArgList::Iter iter(cmd.args); iter; ++iter) {
                const OptionInfo& info = *iter;
                StrSizeT len = info.format(tmp, true).size();
                if (len > maxlen)
                    maxlen = len;
                if (!info.help.empty())
                    found_help = true;
            }

            if (found_help) {
                con_.out << newline_ << "Arguments:" << newline_;
                const StrSizeT help_indent = maxlen + SEP.size() + 1;
                for (typename ArgList::Iter iter(cmd.args); iter; ++iter) {
                    const OptionInfo& info = *iter;
                    con_.out << ' ' << FmtString(info.format(tmp, true), maxlen);
                    if (info.help.empty())
                        con_.out << newline_;
                    else
                        con_.out << SEP << FmtStringWrap(info.help, maxline_ - help_indent).set_indent(help_indent).set_newline(newline_);
                }
            }
        }
    }

    void print_help(const CommandInfo* cur_cmd=NULL) const {
        String tmp;

        con_.out << "Usage: " << progname_;
        print_help_usage(main_, (subcommands_.size() > 0 ? "global_" : ""));

        if (cur_cmd != NULL) {
            con_.out << ' ' << cur_cmd->name;
            print_help_usage(*cur_cmd, "command_");
            con_.out << newline_;
            print_help_args(*cur_cmd);
            return;
        }

        if (subcommands_.size() > 0)
            con_.out << " <command> [args]";
        con_.out << newline_;
        if (description_.size() > 0)
            con_.out << newline_ << FmtStringWrap(description_, maxline_).set_newline(newline_);

        print_help_args(main_, (subcommands_.size() > 0 ? "Global " : ""));

        SubString SEP("   ", 3);
        if (subcommands_.size() > 0) {
            con_.out << newline_ << "Commands:" << newline_;

            StrSizeT maxlen = 0;
            for (List<String>::Iter iter(subcommand_list_); iter; ++iter) {
                const StrSizeT len = iter->size();
                if (len > maxlen)
                    maxlen = len;
            }

            const StrSizeT help_indent = maxlen + SEP.size() + 1;
            for (List<String>::Iter iter(subcommand_list_); iter; ++iter) {
                const CommandInfo& info = *(subcommands_.find(*iter));
                con_.out << ' ' << FmtString(info.name, maxlen);
                if (info.helptext.empty())
                    con_.out << newline_;
                else
                    con_.out << SEP << FmtStringWrap(info.helptext, maxline_ - help_indent).set_indent(help_indent).set_newline(newline_);
            }
        }

        if (epilog_.size() > 0)
            con_.out << newline_ << epilog_ << newline_;
    }

    void print_help_general() const {
        const char* help =
            "\nOptions\n"
            "-------\n\n"
            "Types:\n"
            " * Flag options are boolean and will give an error if a value is supplied\n"
            " * Other options require a value, and in some cases may have multiple values\n"
            "Long options start with a double dash, example: --help\n"
            " * and may include a value using '=', example: --file=myfile\n"
            " * or may give a value with the next argument, example: --file myfile\n"
            "Short options are a single character and start with a single dash, example: -h\n"
            " * and may be combined in standard Unix/Linux fashion, example '-abc' is the same as: -a -b -c\n"
            " * and may include a value using '=', example: -f=myfile\n"
            "   * or may include a value with an additional argument, example: -f myfile\n"
            "   * or may include a value without any separator, example '-fmyfile' is the same as: -f=myfile\n"
            "   * however a separator is required when combining options, example '-abcf=myfile' is the same as: -a -b -c -f=myfile\n"
            " * a short option with a value must be last when combined with flag options, to avoid confusion\n"
            "\nNote that in most cases options may be mixed in with arguments, but option order often does matter,"
            " and repeated options may either accumulate or replace the previous value, depending on the option.\n"
            "\nReset Options\n"
            "-------------\n\n"
            "Any option may be reset (deleted) by prefixing it with '--no-', example '--no-file' deletes option '--file',"
            " and the option may then be set again afterwards.\n"
            "\nSpecial Arguments\n"
            "-----------------\n\n"
            "* A double dash argument (--) stops option processesing and all options after this are treated as raw arguments."
            " This is useful for specifying options that will be passed to another program via arguments\n"
            "* A dash-dot argument (-.) terminates a list of multiple values (rare)\n"
        ;
        con_.out << "Evo C++ Library CommandLine processor version " << EVO_VERSION_STRING << newline_
            << FmtStringWrap(SubString(help), maxline_).set_newline(newline_);
    }
};

/** Process command-line arguments.
 - See CommandLineT
*/
typedef CommandLineT<> CommandLine; 

///////////////////////////////////////////////////////////////////////////////
//@}
}
#endif
