#!/bin/bash

BATH_PATH=$(cd `dirname $0`; pwd)

cd $BATH_PATH

make clean
make -j 4
make install

