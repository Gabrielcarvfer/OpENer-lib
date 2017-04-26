opENer_common_includes()


add_subdirectory(ethIP)
add_subdirectory(deviceNet)

set( CIP_NET_SRC NET_Encapsulation.cpp NET_Connection.cpp NET_NetworkHandler.cpp NET_Endianconv.cpp) #${CIP_NET_ETHIP_SRC} ${CIP_NET_DNET_SRC})

add_library( OpENer_NET STATIC ${CIP_NET_SRC})

if(__WIN32__)
	#if MINGW
	if(MINGW)
		target_link_libraries(OpENer_NET OpENerLib OpENer_CONN CIP_NET_ETHIP CIP_NET_DNET CIP_CLASS00F6_ETHERNETLINK CIP_CLASS0001_IDENTITY ws2_32 wsock32)
	#if Visual Studio
	else ()
		add_definitions(-DVSTUDIO)
		target_link_libraries(OpENer_NET OpENerLib OpENer_CONN CIP_NET_ETHIP CIP_NET_DNET CIP_CLASS00F6_ETHERNETLINK CIP_CLASS0001_IDENTITY)
	endif()
elseif(LINUX)
	target_link_libraries(OpENer_NET OpENerLib OpENer_CONN CIP_NET_ETHIP CIP_NET_DNET CIP_CLASS00F6_ETHERNETLINK CIP_CLASS0001_IDENTITY)
endif()