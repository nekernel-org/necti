#!/bin/sh

THIS_PATH="$(realpath "$0")"
THIS_DIR="$(dirname "$THIS_PATH")"

FILE_LIST="$(find "$THIS_DIR" | grep -E ".*(\.cpp|\.c|\.h|\.inl)$")"

ifneq $DEBUG_FORMAT ""
echo -e "Files found to format = \n\"\"\"\n$FILE_LIST\n\"\"\""
endif

clang-format --verbose -i --style=file $FILE_LIST
