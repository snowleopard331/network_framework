#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/03

import os


# separate: build a analyzable file for every thread in multithread
os.system("valgrind --tool=callgrind --separate-threads=yes  ./auth");
