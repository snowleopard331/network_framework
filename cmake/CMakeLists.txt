CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(server)

SET(EXECUTABLE_OUTPUT_PATH ../bin)

AUX_SOURCE_DIRECTORY(../src SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/share SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/share/log SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/share/policy SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/platform SRC_LIST)
AUX_SOURCE_DIRECTORY(../src/utilities SRC_LIST)

# MESSAGE(${SRC_LIST})

SET(GLOG_INCLUDE_DIR ../dep/glog/include)
SET(BOOST_INCLUDE_DIR ../dep/boost/include)
SET(MY_INCLUDE_DIR ../src/share ../src/platform ../src/utilities)

SET(GLOG_LIB_DIR ../dep/glog/lib)
SET(BOOST_LIB_DIR ../dep/boost/lib)

SET(LIB_LIST boost_thread boost_system glog pthread)

# -rdynamic for stack
ADD_DEFINITIONS(-rdynamic -g -W -Wall -std=c++11)

INCLUDE_DIRECTORIES(${MY_INCLUDE_DIR}  ${GLOG_INCLUDE_DIR} ${BOOST_INCLUDE_DIR})

LINK_DIRECTORIES(${GLOG_LIB_DIR} ${BOOST_LIB_DIR})

ADD_EXECUTABLE(server ${SRC_LIST})

TARGET_LINK_LIBRARIES(server ${LIB_LIST})
