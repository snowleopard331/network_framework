#!/bin/bash

make clean

rm -rvf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile

ccmake ./

cmake ./

