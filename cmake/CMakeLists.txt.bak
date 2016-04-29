CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(evil)

# set project path
SET(PROJECT_PATH /home/yunfei/myworld/network_ramework)

# set executable path
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_PATH}/bin)

# find *.cpp
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/evil SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/evil/network SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share/log SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share/policy SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/platform SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/utilities SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share/database SRC_LIST)

# MESSAGE(${SRC_LIST})

# include *.h
SET(DEP_INCLUDE_DIR ${PROJECT_PATH}/dep/glog/include ${PROJECT_PATH}/dep/boost/include ${PROJECT_PATH}/dep/cryptopp/include ${PROJECT_PATH}/dep/mysql/include)
SET(MY_INCLUDE_DIR ${PROJECT_PATH}/src/share ${PROJECT_PATH}/src/platform ${PROJECT_PATH}/src/utilities ${PROJECT_PATH}/src/evil ${PROJECT_PATH}/src/evil/network)

# all dependent library directorys
SET(DEP_LIB_DIR ${PROJECT_PATH}/dep/glog/lib ${PROJECT_PATH}/dep/boost/lib ${PROJECT_PATH}/dep/cryptopp/lib ${PROJECT_PATH}/dep/mysql/lib)

# all dependent libs list
SET(LIB_LIST boost_thread boost_system glog pthread cryptopp mysqlclient_r)

# compile options,  -rdynamic for stack
ADD_DEFINITIONS(-g -W -Wall -std=c++11)




INCLUDE_DIRECTORIES(${MY_INCLUDE_DIR}  ${DEP_INCLUDE_DIR})

LINK_DIRECTORIES(${DEP_LIB_DIR})

ADD_EXECUTABLE(evil ${SRC_LIST})

TARGET_LINK_LIBRARIES(evil ${LIB_LIST})















