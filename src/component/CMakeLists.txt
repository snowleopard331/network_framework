AUX_SOURCE_DIRECTORY(./network LIB_NET_LIST)
AUX_SOURCE_DIRECTORY(./database LIB_DB_LIST)

INCLUDE_DIRECTORIES(${GLOG_INCLUDE_DIR}  ${BOOST_INCLUDE_DIR} ${CRYPTOPP_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${HIREDIS_INCLUDE_DIR} ${UTILITIES_INCLUDE_DIR} ${SHARE_INCLUDE_DIR} ${NETWORK_INCLUDE_DIR})

SET(LIB_LIST boost_thread boost_system glog pthread cryptopp mysqlclient_r hiredis)

ADD_LIBRARY(network SHARED ${LIB_NET_LIST})
ADD_LIBRARY(network_static STATIC ${LIB_NET_LIST})
SET_TARGET_PROPERTIES(network_static PROPERTIES OUTPUT_NAME "network")

ADD_LIBRARY(database SHARED ${LIB_DB_LIST})
ADD_LIBRARY(database_static STATIC ${LIB_DB_LIST})
SET_TARGET_PROPERTIES(database_static PROPERTIES OUTPUT_NAME "database")

## install libs
INSTALL(TARGETS network network_static database database_static
        LIBRARY DESTINATION network_framework/lib
        ARCHIVE DESTINATION network_framework/lib)
