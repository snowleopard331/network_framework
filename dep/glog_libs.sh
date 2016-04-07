#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

GLOG_PATH=$PROJECT_PATH/network_ramework/dep/glog


#####################################
# check folder
#####################################

if [ -d "glog" ];then
    rm -rv $GLOG_PATH/*
else
    mkdir -vpm 775 $GLOG_PATH
fi

#####################################
# compile libs
#####################################

cd glog-0.3.3/

./configure --prefix=$GLOG_PATH

make;make install

cd -

rm -rv $GLOG_PATH/share

echo -e "\n"
echo "**********************************************************"
echo "*                 glog compile finished                  *"
echo "**********************************************************"
echo -e "\n"
