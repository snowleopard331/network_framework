#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

cd $PROJECT_PATH/network_ramework/cmake

make clean

rm -rvf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile

ccmake ./

cmake ./

cd -
