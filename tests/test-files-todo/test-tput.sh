#!/bin/bash

set -e

TERMINFO_FILE="test"
TERMINFO_STR="str"

if [ -z "$*" ]; then
    echo "Syntax: $0 [-v] <terminfo string value> [<tput arguments..>]"
    exit 1
fi

if [ "$1" == "-v" ]; then
    tic_args=-v4
    shift
fi

# get a temp directory
dir=$(mktemp -d)

# make the terminfo directory layout
mkdir -p "$dir/${TERMINFO_FILE::1}/"

cat <<EOF > "$dir/$TERMINFO_FILE"
$TERMINFO_FILE,
    $TERMINFO_STR=$1,
EOF

# shift the first argument
shift

# cat "$dir/$TERMINFO_FILE"
tic -o "$dir" -x $tic_args "$dir/$TERMINFO_FILE"

# run tput with correct args
TERMINFO="$dir" tput -T "$TERMINFO_FILE" "$TERMINFO_STR" "$@" | cat -A

# newline cause tput wont print one
echo
