#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/03

import os, time


# output redirection
os.system("./boost_libs.py");
time.sleep(3);

os.system("./glog_libs.py");
time.sleep(3);

os.system("./cryptopp_libs.py");
time.sleep(3);

os.system("./mysql_libs.py");
time.sleep(3);

os.system("./redis_libs.py");


print "**********************************************************"
print "*                                                        *"
print "*               All libs compile finished                *"
print "*                                                        *"
print "**********************************************************"
