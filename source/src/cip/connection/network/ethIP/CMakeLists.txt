opENer_common_includes()

set( CIP_NET_ETHIP_SRC NET_EthIP_Encap.cpp eip_endianconv.cpp ../../../CIP_Objects/CIP_00F6_EthernetLink/CIP_EthIP_Link.cpp ../../../CIP_Objects/CIP_00F5_TCPIP_Interface/CIP_EthIP_Interface.cpp)
add_library( CIP_NET_ETHIP STATIC ${CIP_NET_ETHIP_SRC})

if(WIN32)
	#if MINGW
	if(MINGW)
		target_link_libraries(CIP_NET_ETHIP ws2_32 wsock32)
	#if Visual Studio
	else ()
		add_definitions(-DVSTUDIO)
		target_link_libraries(CIP_NET_ETHIP)
	endif()
elseif(LINUX)
	target_link_libraries(CIP_NET_ETHIP)
endif()