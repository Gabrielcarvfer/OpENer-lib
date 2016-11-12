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

set( CIP_SRC appcontype.cpp cipassembly.cpp connection_stack/cipclass3connection.cpp connection_stack/cipcommon.cpp connection_stack/cipconnectionmanager.cpp ciperror.h network_stack/ethernetip_net/cipethernetlink.cpp cipidentity.cpp connection_stack/cipioconnection.cpp connection_stack/cipmessagerouter.cpp class_stack/CIP_Class.cpp network_stack/ethernetip_net/tcpip_link/ciptcpipinterface.cpp ciptypes.h cpf.cpp class_stack/CIP_Attribute.cpp class_stack/CIP_Attribute.h class_stack/CIP_Service.cpp class_stack/CIP_Service.h network_stack/CIP_Network.cpp network_stack/CIP_Network.h Opener_Interface.cpp Opener_Interface.h Opener_ExplicitConnection.cpp Opener_ExplicitConnection.h Opener_IOConnection.cpp Opener_IOConnection.h)

add_library( CIP ${CIP_SRC} ${UTILS_SRC})
