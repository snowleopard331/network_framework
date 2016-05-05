CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(server)

## set project path
SET(PROJECT_PATH /home/yunfei/myworld/network_ramework)

## set executable path
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_PATH}/bin)



#################################### find *.cpp #############################################

## common
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share             COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share/log         COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share/policy      COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/platform          COMMON_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/utilities         COMMON_SRC_LIST)
## evil
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/evil              EVIL_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/evil/network      EVIL_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share/database    EVIL_SRC_LIST)
## auth
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/auth              AUTH_SRC_LIST)
AUX_SOURCE_DIRECTORY(${PROJECT_PATH}/src/share/database    AUTH_SRC_LIST)



## include *.h
SET(DEP_INCLUDE_DIR ${PROJECT_PATH}/dep/glog/include 
                    ${PROJECT_PATH}/dep/boost/include 
                    ${PROJECT_PATH}/dep/cryptopp/include 
                    ${PROJECT_PATH}/dep/mysql/include)

SET(MY_INCLUDE_DIR  ${PROJECT_PATH}/src/share 
                    ${PROJECT_PATH}/src/platform 
                    ${PROJECT_PATH}/src/utilities 
                    ${PROJECT_PATH}/src/evil 
                    ${PROJECT_PATH}/src/evil/network 
                    ${PROJECT_PATH}/src/auth)


## all dependent library directorys
SET(DEP_LIB_DIR ${PROJECT_PATH}/dep/glog/lib 
                ${PROJECT_PATH}/dep/boost/lib 
                ${PROJECT_PATH}/dep/cryptopp/lib 
                ${PROJECT_PATH}/dep/mysql/lib)

### dependent libs list
SET(LIB_LIST boost_thread boost_system glog pthread cryptopp mysqlclient_r)


## compile options,  -rdynamic for stack, -fdiagnostics-color=auto need gcc--version more than 4.9.0
ADD_DEFINITIONS(-g -W -Wall -std=c++11 -fdiagnostics-color=auto)


## include all header files directory
INCLUDE_DIRECTORIES(${MY_INCLUDE_DIR}  ${DEP_INCLUDE_DIR} ${EVIL_INCLUDE_DIR} ${AUTH_INCLUDE_DIR})
LINK_DIRECTORIES(${DEP_LIB_DIR})



############################### excute bin file #######################################

## evil
ADD_EXECUTABLE(evil ${COMMON_SRC_LIST} ${EVIL_SRC_LIST})
TARGET_LINK_LIBRARIES(evil ${LIB_LIST})

## auth
ADD_EXECUTABLE(auth ${COMMON_SRC_LIST} ${AUTH_SRC_LIST})
TARGET_LINK_LIBRARIES(auth ${LIB_LIST})

## add new server


############################## install ######################################

SET(CMAKE_INSTALL_PREFIX ${PROJECT_PATH}/bin)
INSTALL(TARGETS   evil
	DESTINATION evilServer)

INSTALL(TARGETS   auth
        DESTINATION authServer)











