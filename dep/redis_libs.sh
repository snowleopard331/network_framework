#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

REDIS_PATH=$PROJECT_PATH/network_ramework/dep/hiredis
REDIS_INSTALL_PATH=/usr/local/redis

cd $PROJECT_PATH/network_ramework/dep


#####################################
# check folder
#####################################

if [ -d "hiredis" ];then
    rm -rv $REDIS_PATH/*
else
    mkdir -vpm 775 $REDIS_PATH
fi

#####################################
# compile libs
#####################################

cd redis-3.2.1//deps/hiredis

make -j 4
make install PREFIX=$REDIS_PATH
make clean

cd -


rm -rv $REDIS_PATH/share

echo -e "\n"
echo "**********************************************************"
echo "*                 hiredis compile finished                  *"
echo "**********************************************************"
echo -e "\n"
