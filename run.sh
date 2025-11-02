#!/bin/bash
set -e
cmake -S . -B build
(cd build && make)
"./build/mp" "$@"
