#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/03

import os


os.system("valgrind --tool=memcheck --log-file=./log/memeryCheck.log --leak-check=full --show-reachable=yes --trace-children=yes  ./auth");
