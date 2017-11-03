#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/02

import os

curPath = os.getcwd();
libDir = "boost";
libSrcDir = "boost_1_60_0";

if os.path.exists(libDir):
    os.system("rm -rfv " + libDir);
else:
    os.system("mkdir -vpm 775 " + libDir);

os.chdir(libSrcDir);

desDir = curPath + os.sep + libDir;
os.system("./bootstrap.sh --prefix=" + desDir);
os.system("./b2 install --with-thread --with-system");

print "**********************************************************"
print "*                 boost compile finished                 *"
print "**********************************************************"
