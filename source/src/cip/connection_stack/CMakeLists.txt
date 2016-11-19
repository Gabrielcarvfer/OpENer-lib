#######################################
# CIP library                         #
#######################################

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platform-specific includes      #
#######################################
opener_platform_support("INCLUDES")

set( CIP_CONN_SRC CIP_Connection.cpp cipclass3connection.cpp cipcommon.cpp cipioconnection.cpp cipmessagerouter.cpp)

