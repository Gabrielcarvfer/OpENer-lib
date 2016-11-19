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

set( CIP_NET_SRC ethernetip_net/cipethernetlink.cpp network_stack/ethernetip_net/tcpip_link/ciptcpipinterface.cpp ciptypes.h network_stack/CIP_Network.cpp network_stack/CIP_Network.h)

