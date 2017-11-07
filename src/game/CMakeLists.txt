AUX_SOURCE_DIRECTORY(. GAME_SRC_LIST)

### dependent libs list
SET(LIB_LIST boost_thread boost_system glog pthread cryptopp mysqlclient_r hiredis share database network)

## include all header files directory
INCLUDE_DIRECTORIES(${GLOG_INCLUDE_DIR}  ${BOOST_INCLUDE_DIR} ${CRYPTOPP_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${HIREDIS_INCLUDE_DIR} ${GAME_INCLUDE_DIR} ${SHARE_INCLUDE_DIR} ${PLATFORM_INCLUDE_DIR} ${UTILITIES_INCLUDE_DIR} ${DATABASE_INCLUDE_DIR} ${NETWORK_INCLUDE_DIR})

LINK_DIRECTORIES(${GLOG_LIB_DIR}  ${BOOST_LIB_DIR} ${CRYPTOPP_LIB_DIR} ${MYSQL_LIB_DIR} ${HIREDIS_LIB_DIR} ${MY_LIB_DIR})

## game
ADD_EXECUTABLE(game ${COMMON_SRC_LIST} ${GAME_SRC_LIST})
TARGET_LINK_LIBRARIES(game ${LIB_LIST})

## add new server


############################## install ######################################

INSTALL(TARGETS game DESTINATION network_framework/bin/game_server)
