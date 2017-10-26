CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(server)

## set executable path
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

############################### source files ####################################

## common
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/share             COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/share/log         COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/share/policy      COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/platform          COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/utilities         COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/share/database    COMMON_SRC_LIST)


## include head file folder of 3rd lib
SET(GLOG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dep/glog/include)
SET(BOOST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dep/boost/include)
SET(CRYPTOPP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dep/cryptopp/include) 
SET(MYSQL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dep/mysql/include)
SET(HIREDIS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dep/hiredis/include)

## common head file folder
SET(SHARE_INCLUDE_DIR  ${PROJECT_SOURCE_DIR}/src/share) 
SET(PLATFORM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/platform)
SET(UTILITIES_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/utilities)

## project head file folder
SET(GAME_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/evil ${PROJECT_SOURCE_DIR}/src/evil/network)
SET(AUTH_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/auth)


## all dependent library directorys
SET(GLOG_LIB_DIR ${PROJECT_SOURCE_DIR}/dep/glog/lib)
SET(BOOST_LIB_DIR ${PROJECT_SOURCE_DIR}/dep/boost/lib)
SET(CRYPTOPP_LIB_DIR ${PROJECT_SOURCE_DIR}/dep/cryptopp/lib)
SET(MYSQL_LIB_DIR ${PROJECT_SOURCE_DIR}/dep/mysql/lib)
SET(HIREDIS_LIB_DIR ${PROJECT_SOURCE_DIR}/dep/hiredis/lib)


## compile options,  -rdynamic for stack, -fdiagnostics-color=auto need gcc--version more than 4.9.0
ADD_DEFINITIONS(-g -W -Wall -std=c++11 -fdiagnostics-color=auto)

ADD_SUBDIRECTORY(src)

