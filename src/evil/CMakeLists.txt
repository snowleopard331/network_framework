CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(server)

AUX_SOURCE_DIRECTORY(. GAME_SRC_LIST)
AUX_SOURCE_DIRECTORY(./network GAME_SRC_LIST)

### dependent libs list
SET(LIB_LIST boost_thread boost_system glog pthread cryptopp mysqlclient_r hiredis)

## include all header files directory
INCLUDE_DIRECTORIES(${GLOG_INCLUDE_DIR}  ${BOOST_INCLUDE_DIR} ${CRYPTOPP_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${HIREDIS_INCLUDE_DIR} ${GAME_INCLUDE_DIR} ${SHARE_INCLUDE_DIR} ${PLATFORM_INCLUDE_DIR} ${UTILITIES_INCLUDE_DIR})
LINK_DIRECTORIES(${GLOG_LIB_DIR}  ${BOOST_LIB_DIR} ${CRYPTOPP_LIB_DIR} ${MYSQL_LIB_DIR} ${HIREDIS_LIB_DIR})

## game
ADD_EXECUTABLE(game ${COMMON_SRC_LIST} ${GAME_SRC_LIST})
TARGET_LINK_LIBRARIES(game ${LIB_LIST})

## add new server


############################## install ######################################

INSTALL(TARGETS game DESTINATION network_ramework/bin/game_server)











