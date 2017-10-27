#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld

MYSQL_PATH=$PROJECT_PATH/network_ramework/dep/mysql
MYSQL_INSTALL_PATH=/usr/local/mysql

cd $PROJECT_PATH/network_ramework/dep


#####################################
# check folder
#####################################

if [ -d "mysql" ];then
    rm -rv $MYSQL_PATH/*
else
    mkdir -vpm 775 $MYSQL_PATH
fi

#####################################
# compile libs
#####################################

cd mysql-5.6.30

cmake \
-DCMAKE_INSTALL_PREFIX=$MYSQL_INSTALL_PATH \
-DMYSQL_UNIX_ADDR=/usr/local/mysql/mysql.sock \
-DDEFAULT_CHARSET=utf8 \
-DDEFAULT_COLLATION=utf8_general_ci \
-DWITH_MYISAM_STORAGE_ENGINE=1 \
-DWITH_INNOBASE_STORAGE_ENGINE=1 \
-DWITH_ARCHIVE_STORAGE_ENGINE=1 \
-DWITH_BLACKHOLE_STORAGE_ENGINE=1 \
-DWITH_MEMORY_STORAGE_ENGINE=1 \
-DWITH_READLINE=1 \
-DENABLED_LOCAL_INFILE=1 \
-DMYSQL_DATADIR=/home/mysql/data \
-DMYSQL_USER=mysql \
-DMYSQL_TCP_PORT=3306 \
-DENABLE_DOWNLOADS=1

make -j 4
make install
# make clean

cd -

cp -rfv $MYSQL_INSTALL_PATH/lib $MYSQL_PATH/
cp -rfv $MYSQL_INSTALL_PATH/include $MYSQL_PATH/

rm -rv $MYSQL_PATH/share

echo -e "\n"
echo "**********************************************************"
echo "*                 mysql compile finished                  *"
echo "**********************************************************"
echo -e "\n"
