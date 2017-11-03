#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/02

import os


# ccmake 和 cmake 的区别在于前者提供了一个交互式的界面
# ccmake -DCMAKE_INSTALL_PREFIX=/home/yunfei/myworld ..

os.system("make clean");
os.system("cmake -DCMAKE_INSTALL_PREFIX=/home/yunfei/myworld ..");
