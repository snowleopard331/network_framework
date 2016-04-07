#!/bin/bash

# do ready work
PROJECT_PATH=/home/yunfei/myworld

BOOST_PATH=$PROJECT_PATH/network_ramework/dep/boost
GLOG_PATH=$PROJECT_PATH/network_ramework/dep/glog
CTYPTOPP_PATH=$PROJECT_PATH/network_ramework/dep/cryptopp

if [ -d "boost" ];then
    rm -rv $BOOST_PATH/*
else
    mkdir -vpm 775 $BOOST_PATH
fi

if [ -d "glog" ];then
    rm -rv $GLOG_PATH/*
else
    mkdir -vpm 775 $GLOG_PATH
fi

if [ -d "cryptopp" ];then
    rm -rv $CTYPTOPP_PATH/*
else
    mkdir -vpm 775 $CTYPTOPP_PATH
fi


chmod 775 -R $PROJECT_PATH/network_ramework/dep


# compile libs
cd boost_1_60_0/

./bootstrap.sh --prefix=$BOOST_PATH

./bjam install

cd -

echo -e "\n"
echo "**********************************************************"
echo "*                 boost compile finished                 *"
echo "**********************************************************"
echo -e "\n"

cd glog-0.3.3/

./configure --prefix=$GLOG_PATH

make;make install

cd -

echo -e "\n"
echo "**********************************************************"
echo "*                 glog compile finished                  *"
echo "**********************************************************"
echo -e "\n"

rm -rv $GLOG_PATH/share

cd cryptopp/

make;make libcryptopp.so
make install PREFIX=$CTYPTOPP_PATH
make clean

cd -

echo -e "\n"
echo "**********************************************************"
echo "*               cryptopp compile finished                 *"
echo "**********************************************************"
echo -e "\n"









