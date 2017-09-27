// Evo C++ Library
/* Copyright (c) 2016 Justin Crowell
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
///////////////////////////////////////////////////////////////////////////////
/** \file config.h Evo Configuration helpers. */
#pragma once
#ifndef INCL_evo_config_h
#define INCL_evo_config_h

// Includes
#include "maphash.h"
#include "ptr.h"
#include "strtok.h"

// Namespace: evo
namespace evo {

/** \addtogroup EvoContainers */
//@{

///////////////////////////////////////////////////////////////////////////////

/**
*/
class Config
{
public:
    class ArgParse {
    private:
        enum Type {
            atFlag,
            atStore,
            atAppend,
            atCount
        };

        struct Arg {
            Type   type;
            String dest;
            String defval;
            String help;
            String helpname;
            int    valcount;

            Arg() : valcount(0)
                { }
            Arg(Type type, const String& dest, const String& help, const String& defval) :
                type(type),
                dest(dest),
                help(help),
                valcount(0)
                { }
            Arg(Type type, const String& dest, const String& help, const String& helpname, const String& defval, int valcount) :
                type(type),
                dest(dest),
                defval(defval),
                help(help),
                helpname(helpname),
                valcount(valcount)
                { }
            Arg(const Arg& src) :
                type(src.type),
                dest(src.dest),
                defval(src.defval),
                help(src.help),
                helpname(src.helpname),
                valcount(src.valcount)
                { }
            Arg& operator=(const Arg& src) {
                type     = src.type;
                dest     = src.dest;
                defval   = src.defval;
                help     = src.help;
                helpname = src.helpname;
                valcount = src.valcount;
                return *this;
            }
        };

        String description;
        String version;
        MapHash<String,SharedPtr<Arg> > options;
        List<Arg>                       args;

    public:
        // TODO: support predefined choices

        static const int VALUES_REMAIN = -1;    ///< All remaining options and arguments, useful for passing args to other tools
        static const int VALUES_ANY    = -2;    ///< Any number of values
        static const int VALUES_MIN1   = -3;    ///< Any number of values but at least 1

        ArgParse(const String& description, const String& version) :
            description(description), version(version)
            { }

        void addflag(const String& names, const String& dest, const String& help, bool defval=false) {
            SharedPtr<Arg> arg = new Arg(atFlag, dest, help, defval?"1":"0");
            StrTok tok(names);
            while (tok.next(','))
                options[tok.value()] = arg;
        }

        void addopt(const String& names, const String& dest, const String& help, const String& helpname, const String& defval=String(), int values=1) {
            SharedPtr<Arg> arg = new Arg(atStore, dest, help, helpname, defval, values);
            StrTok tok(names);
            while (tok.next(','))
                options[tok.value()] = arg;
        }

        void addopt_append(const String& names, const String& dest, const String& help, const String& helpname, const String& defval=String(), int values=1) {
            SharedPtr<Arg> arg = new Arg(atAppend, dest, help, helpname, defval, values);
            StrTok tok(names);
            while (tok.next(','))
                options[tok.value()] = arg;
        }

        void addpos(const String& dest, const String& help, const String& helpname, const String& defval=String(), int valcount=1) {
            Arg& arg = args.addnew();
            arg.dest     = dest;
            arg.help     = help;
            arg.helpname = helpname;
            arg.defval   = defval;
            arg.valcount = valcount;
        }

        template<class TMap>
        void parse(TMap& map, int argc, char** argv) {
            // TODO
        }
    };
};

///////////////////////////////////////////////////////////////////////////////
//@}
} // Namespace: evo
#endif
