#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

CTYPTOPP_PATH=$PROJECT_PATH/network_ramework/dep/cryptopp


#####################################
# check folder
#####################################

if [ -d "cryptopp" ];then
    rm -rv $CTYPTOPP_PATH/*
else
    mkdir -vpm 775 $CTYPTOPP_PATH
fi


#####################################
# compile libs
#####################################

cd cryptopp-master/

make;make libcryptopp.so
make install PREFIX=$CTYPTOPP_PATH
make clean

cd -

rm -rv $CTYPTOPP_PATH/share $CTYPTOPP_PATH/bin

echo -e "\n"
echo "**********************************************************"
echo "*               cryptopp compile finished                 *"
echo "**********************************************************"
echo -e "\n"
