#!/bin/bash

BATH_PATH=$(cd `dirname $0`; pwd)

cd $BATH_PATH/../build

make -j 4;make install

cd -

