#!/bin/bash
set -e

build=()
run=()
t=false

for arg in "$@"; do
    if $t; then
        run+=("$arg")
    elif [[ "$arg" = '--' ]]; then
        t=true
    else 
        build+=("$arg")
    fi
done

cmake ${build[@]} -S . -B build
(cd build && make)
"./build/mc" ${run[@]} < /dev/stdin
