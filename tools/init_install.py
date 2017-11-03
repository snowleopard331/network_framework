#!/usr/bin/python
# -*- coding: UTF-8 -*-

# author by : Jovi
# email     : snowleopard331@163.com
# date      : 2017/11/03

# please installing git by developer and running this script by root access
# system    : Fedora23 64bit


import os

# update system to latest version
os.system("dnf -y update");

# install necenary tools
os.system("dnf -y install gcc gcc-c++ cmake samba gdb valgrind");

# install mysql necessary
os.system("dnf -y install ncurses-devel make perl  perl-Module-Install.noarch");
