#!/bin/bash
rm -rf builddir/ bin/
mkdir -p builddir/
cmake -B builddir -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cp ./builddir/compile_commands.json .
rm -rf builddir/ bin/
cmake -B builddir
