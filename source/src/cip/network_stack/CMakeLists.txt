
add_subdirectory(ethernetip_net)
#add_subdirectory(devicenet_net)

opener_common_includes()

set( CIP_NET_SRC ${CIP_NET_ETHIP_SRC} NET_Connection.cpp NET_NetworkHandler.cpp) #${CIP_NET_DNET_SRC}

add_library( OpENer_NET ${CIP_NET_SRC})
