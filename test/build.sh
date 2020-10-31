#!/bin/bash

TARGET_DIR=./output
ARGS="-x -v4"

for i in ./*.terminfo; do
    # only process files
    [ ! -f "$i" ] && continue

    tic -o "$TARGET_DIR" "$i" $ARGS "$@"
done
