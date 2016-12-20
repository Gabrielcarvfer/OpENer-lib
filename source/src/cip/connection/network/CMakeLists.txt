opener_common_includes()


add_subdirectory(ethIP)
#add_subdirectory(deviceNet)

set( CIP_NET_SRC NET_Encapsulation.cpp NET_Connection.cpp NET_NetworkHandler.cpp NET_Endianconv.cpp) #${CIP_NET_ETHIP_SRC} ${CIP_NET_DNET_SRC})

add_library( OpENer_NET STATIC ${CIP_NET_SRC})

target_link_libraries(OpENer_NET CIP_NET_ETHIP)