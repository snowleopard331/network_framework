#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

BOOST_PATH=$PROJECT_PATH/network_ramework/dep/boost


#####################################
# check folder
#####################################

if [ -d "boost" ];then
    rm -rv $BOOST_PATH/*
else
    mkdir -vpm 775 $BOOST_PATH
fi


#####################################
# compile libs
#####################################

cd boost_1_60_0/

./bootstrap.sh --prefix=$BOOST_PATH

./bjam install --with-thread --with-system

cd -

echo -e "\n"
echo "**********************************************************"
echo "*                 boost compile finished                 *"
echo "**********************************************************"
echo -e "\n"
