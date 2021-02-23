#!/bin/bash

mkdir -p build
#remove version info from makefiles. 
#  flags.make contains the version info
cd build
find . -name "flags.make" -delete
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi-8-2019-q3-update.cmake
make canable_fw

