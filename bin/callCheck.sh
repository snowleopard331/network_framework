#!/bin/bash

# separate: build a analyzable file for every thread in multithread
valgrind --tool=callgrind --separate-threads=yes  ./server
