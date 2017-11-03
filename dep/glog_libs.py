#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/02

import os

curPath = os.getcwd();
libDir = "glog";
libSrcDir = "glog-0.3.3";

if os.path.exists(libDir):
    os.system("rm -rfv " + libDir);
else:
    os.system("mkdir -vpm 775 " + libDir);

os.chdir(libSrcDir);

desDir = curPath + os.sep + libDir;
os.system("./configure --prefix=" + desDir);
os.system("make -j 4; make install");

print "**********************************************************"
print "*                 glog compile finished                  *"
print "**********************************************************"
