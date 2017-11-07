AUX_SOURCE_DIRECTORY(. AUTH_SRC_LIST)

### dependent libs list
SET(LIB_LIST boost_thread boost_system glog pthread cryptopp mysqlclient_r hiredis share database network)

## include all header files directory
INCLUDE_DIRECTORIES(${GLOG_INCLUDE_DIR}  ${BOOST_INCLUDE_DIR} ${CRYPTOPP_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${HIREDIS_INCLUDE_DIR} ${AUTH_INCLUDE_DIR} ${SHARE_INCLUDE_DIR} ${PLATFORM_INCLUDE_DIR} ${UTILITIES_INCLUDE_DIR} ${DATABASE_INCLUDE_DIR})
LINK_DIRECTORIES(${GLOG_LIB_DIR}  ${BOOST_LIB_DIR} ${CRYPTOPP_LIB_DIR} ${MYSQL_LIB_DIR} ${MY_LIB_DIR})

## auth
ADD_EXECUTABLE(auth ${COMMON_SRC_LIST} ${AUTH_SRC_LIST})
TARGET_LINK_LIBRARIES(auth ${LIB_LIST})



############################## install ######################################

INSTALL(TARGETS auth DESTINATION network_framework/bin/auth_server)
