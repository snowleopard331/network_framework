CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(server)

SET(EXECUTABLE_OUTPUT_PATH ../bin)

AUX_SOURCE_DIRECTORY(../src SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/share SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/share/log SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/share/policy SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/platform SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/utilities SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/share/database SRC_LIST)

# MESSAGE(${SRC_LIST})

SET(DEP_INCLUDE_DIR ../dep/glog/include ../dep/boost/include ../dep/cryptopp/include ../dep/mysql/include)
SET(MY_INCLUDE_DIR ../src/share ../src/platform ../src/utilities)

SET(DEP_LIB_DIR ../dep/glog/lib ../dep/boost/lib ../dep/cryptopp/lib ../dep/mysql/lib)

SET(LIB_LIST boost_thread boost_system glog pthread cryptopp mysqlclient_r)

# -rdynamic for stack
ADD_DEFINITIONS(-g -W -Wall -std=c++11)

INCLUDE_DIRECTORIES(${MY_INCLUDE_DIR}  ${DEP_INCLUDE_DIR})

LINK_DIRECTORIES(${DEP_LIB_DIR})

ADD_EXECUTABLE(server ${SRC_LIST})

TARGET_LINK_LIBRARIES(server ${LIB_LIST})
