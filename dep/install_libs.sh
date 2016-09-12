#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

CTYPTOPP_PATH=$PROJECT_PATH/network_ramework/dep/cryptopp
BOOST_PATH=$PROJECT_PATH/network_ramework/dep/boost
GLOG_PATH=$PROJECT_PATH/network_ramework/dep/glog
MYSQL_PATH=$PROJECT_PATH/network_ramework/dep/mysql
HIREDIS_PATH=$PROJECT_PATH/network_ramework/dep/hiredis

TARGET_PATH=/lib64

if [ -d $TARGET_PATH ];then
    cp -fv $BOOST_PATH/lib/libboost_*  $TARGET_PATH
    cp -fv $GLOG_PATH/lib/libglog.*  $TARGET_PATH
    cp -fv $CTYPTOPP_PATH/lib/libcryptopp.* $TARGET_PATH
    cp -fv $MYSQL_PATH/lib/libmysql* $TARGET_PATH
    cp -fv $HIREDIS_PATH/lib/libhiredis.* $TARGET_PATH

    echo -e "\n"
    echo "**********************************************************"
    echo "*                install    successfully                 *"
    echo "**********************************************************"
    echo -e "\n"

else

    echo -e "\n"
    echo "**********************************************************"
    echo "*                install    faild                        *"
    echo "**********************************************************"
    echo -e "\n"

fi


## do this by root
