#!/bin/bash
set -e

function die {
    echo "$@" 1>&2
    exit 1
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    echo "Usage: $(basename "$0") FILE [[CC] [CCFLAGS...]]"
    exit 0
fi

[[ $1 != "" ]] || die "Expected argument, try -h for help"
filename="$1"
name="${filename%.*}"
[[ "$filename" != "$name" ]] || filename="$name.cpp" 
[ -f "$filename" ] || die "Can't find file: $filename"
shift || true

cc="$1"
shift || true
for ccname in g++ clang++; do
    [ -z "$cc" ] || break
    cc=$(which "$ccname" 2> /dev/null)
done
[ -n "$cc" ] || die "Can't find a compiler, please specify one, try -h for help"

(set -x; "$cc" -O3 -std=c++11 -I.. -o "$name" "$filename" -pthread "$@")

echo ...

"./$name"
