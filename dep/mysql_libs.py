#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/02

import os

curPath = os.getcwd();
libDir = "mysql";
libSrcDir = "mysql-5.6.30";
mysqlInstallPath = "/usr/local/mysql";

if os.path.exists(libDir):
    os.system("rm -rfv " + libDir + os.sep + "*");
else:
    os.system("mkdir -vpm 775 " + libDir);

os.chdir(libSrcDir);

compileMysqlCmd = "cmake \
-DCMAKE_INSTALL_PREFIX=" + mysqlInstallPath + "\
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
-DENABLE_DOWNLOADS=1"

desDir = curPath + os.sep + libDir;
os.system(compileMysqlCmd);
os.system("make -j 4; make install");
os.system("make clean");

os.system("cp -rfv " + mysqlInstallPath + os.sep + "lib "+ mysqlInstallPath + os.sep + "include " + desDir + os.sep);


print "**********************************************************"
print "*                 mysql compile finished                  *"
print "**********************************************************"
