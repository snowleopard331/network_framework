#!/bin/bash


valgrind --tool=memcheck --log-file=./log/memeryCheck.log --leak-check=full --show-reachable=yes --trace-children=yes  ./evil
