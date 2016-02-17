CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(server)

AUX_SOURCE_DIRECTORY(./src SRC_LIST)
AUX_SOURCE_DIRECTORY(./src/share SRC_LIST)

SET(GLOG_INCLUDE_DIR ./dep/glog/include)
SET(BOOST_INCLUDE_DIR ./dep/boost/include)

SET(GLOG_LIB_DIR ./dep/glog/lib)
SET(BOOST_LIB_DIR ./dep/boost/lib)

#SET(LIB_LIST )

INCLUDE_DIRECTORIES(./src/share ${GLOG_INCLUDE_DIR} ${BOOST_INCLUDE_DIR})

ADD_EXECUTABLE(server ${SRC_LIST})

#TARGET_LINK_LIBRARIES()