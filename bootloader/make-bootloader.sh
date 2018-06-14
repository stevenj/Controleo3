#!/bin/sh
ARM_GCC_PATH="$1"
shift 1

cd uf2-samdx1
PATH="$PATH":"$ARM_GCC_PATH" make BOARD=controleo3 UF2_VERSION_BASE="git" "$@"
