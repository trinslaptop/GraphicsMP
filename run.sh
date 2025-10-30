#!/bin/bash
set -e
PROGRAM="$(basename "$(pwd)")"
cmake -S . -B build
(cd build && make)
"./build/$PROGRAM"
