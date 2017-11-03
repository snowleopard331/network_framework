#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/03

import os


os.chdir("./auth_server");
os.system("./auth");

os.chdir("../game_server");
os.system("./game");
