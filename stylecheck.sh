#!/bin/bash

CHECKPATCH_FLAGS='--no-tree --terse --show-types --ignore NEW_TYPEDEFS --ignore AVOID_EXTERNS --ignore LINE_CONTINUATIONS --ignore SINGLE_STATEMENT_DO_WHILE_MACRO'

S=0

echo 'Checking main...'
tools/checkpatch.pl $CHECKPATCH_FLAGS --file include/*.h
S=$(($S + $?))
tools/checkpatch.pl $CHECKPATCH_FLAGS --file src/*.c
S=$(($S + $?))



exit $S
