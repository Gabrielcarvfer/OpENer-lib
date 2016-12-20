opener_common_includes()


add_subdirectory(ethernetip_net)
#add_subdirectory(devicenet_net)

set( CIP_NET_SRC NET_Encapsulation.h NET_Connection.cpp NET_NetworkHandler.cpp) #${CIP_NET_ETHIP_SRC} ${CIP_NET_DNET_SRC})

add_library( OpENer_NET ${CIP_NET_SRC})

target_link_libraries(OpENer_NET OpENer_CLASS CIP_NET_ETHIP)