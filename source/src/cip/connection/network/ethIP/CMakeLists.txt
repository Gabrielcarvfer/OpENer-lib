opENer_common_includes()

set( CIP_NET_ETHIP_SRC NET_EthIP_Encap.cpp eip_endianconv.cpp NET_EthIP_Link.cpp tcpip/NET_EthIP_Interface.cpp)
add_library( CIP_NET_ETHIP STATIC ${CIP_NET_ETHIP_SRC})

target_link_libraries(CIP_NET_ETHIP ws2_32 wsock32)