#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

cd $PROJECT_PATH/network_ramework/

pwd

git pull

cd $PROJECT_PATH/network_ramework/cmake

make clean;make

cd $PROJECT_PATH/network_ramework/bin
