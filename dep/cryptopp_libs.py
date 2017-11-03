#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/02

import os

curPath = os.getcwd();
libDir = "cryptopp";
libSrcDir = "cryptopp-master";

if os.path.exists(libDir):
    os.system("rm -rfv " + libDir);
else:
    os.system("mkdir -vpm 775 " + libDir);

os.chdir(libSrcDir);

desDir = curPath + os.sep + libDir;
os.system("make -j 4; make libcryptopp.so");
os.system("make install PREFIX=" + desDir);

print "**********************************************************"
print "*               cryptopp compile finished                 *"
print "**********************************************************"
