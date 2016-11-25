
add_subdirectory(ethernetip_net)
#add_subdirectory(devicenet_net)

opener_common_includes()

set( CIP_NET_SRC ${CIP_NET_ETHIP_SRC} CIP_Network.cpp) #{CIP_NET_DEVNT_SRC}
