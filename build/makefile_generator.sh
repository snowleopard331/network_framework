#!/bin/bash

# PROJECT_PATH=/home/yunfei/myworld
BATH_PATH=$(cd `dirname $0`; pwd)
echo $BATH_PATH

cd $BATH_PATH

make clean

rm -rvf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile src

# ccmake 和 cmake 的区别在于前者提供了一个交互式的界面
# ccmake -DCMAKE_INSTALL_PREFIX=/home/yunfei/myworld ..

cmake -DCMAKE_INSTALL_PREFIX=/home/yunfei/myworld ..

cd -
