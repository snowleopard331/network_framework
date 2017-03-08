#!/bin/bash

PROJECT_PATH=/home/yunfei/myworld


# boot sequence

cd $PROJECT_PATH/network_ramework/bin/authServer
./start.sh

sleep 1

cd $PROJECT_PATH/network_ramework/bin/evilServer
./start.sh
