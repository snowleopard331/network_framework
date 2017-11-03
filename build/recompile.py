#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/02

import sys,os


if (len(sys.argv) >= 2) and (sys.argv[1].isdigit()):
    cpu_core = sys.argv[1];

    if (int(cpu_core) > 16) or (int(cpu_core) <= 2):
        print '\033[1;31;40m';   # set stdoutput color is red
        print "[WARNING]:compile cpu num " + str(cpu_core) + " may affect the efficiency of compling";
        print '\033[0m';         # set stdoutput color is default
else:
    cpu_core = 4;

print "multi-thread num: " + str(cpu_core);

os.system("make clean");
cmd = "make -j " + str(cpu_core);
os.system(cmd);
os.system("make install");
