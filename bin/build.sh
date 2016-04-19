#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

cd $PROJECT_PATH/network_ramework/cmake

./remake.sh

./compile.sh

cd $PROJECT_PATH/network_ramework/bin

./start.sh
