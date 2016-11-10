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

set( CIP_SRC appcontype.cpp cipassembly.cpp cipclass3connection.cpp cipcommon.cpp cipconnectionmanager.cpp ciperror.h cipethernetlink.cpp cipidentity.cpp cipioconnection.cpp cipmessagerouter.cpp cip_class.cpp ciptcpipinterface.cpp ciptypes.h cpf.cpp)

add_library( CIP ${CIP_SRC} ${UTILS_SRC})
