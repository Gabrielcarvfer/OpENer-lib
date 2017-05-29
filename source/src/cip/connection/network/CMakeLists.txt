opENer_common_includes()

set( CIP_NET_SRC
		NET_Encapsulation.cpp
		NET_Connection.cpp
		NET_NetworkHandler.cpp
		NET_Endianconv.cpp
		./ethIP/NET_EthIP_Encap.cpp
		./ethIP/eip_endianconv.cpp
		../CIP_CommonPacket.cpp)
		#${CIP_NET_DNET_SRC})

add_library( OpENer_NET STATIC ${CIP_NET_SRC})

if(__WIN32__)
	#if MINGW
	if(MINGW)
		target_link_libraries(OpENer_NET CIP_NET_ETHIP OpENerLib OpENer_CONN CIP_CLASS00F6_ETHERNETLINK CIP_CLASS0001_IDENTITY ws2_32 wsock32)# CIP_NET_DNET
	#if Visual Studio
	else ()
		add_definitions(-DVSTUDIO)
		target_link_libraries(OpENer_NET CIP_NET_ETHIP OpENerLib OpENer_CONN CIP_CLASS00F6_ETHERNETLINK CIP_CLASS0001_IDENTITY)#CIP_NET_ETHIP CIP_NET_DNET
	endif()
elseif(LINUX)
	target_link_libraries(OpENer_NET CIP_NET_ETHIP OpENerLib OpENer_CONN  CIP_CLASS00F6_ETHERNETLINK CIP_CLASS0001_IDENTITY)#CIP_NET_ETHIP CIP_NET_DNET
endif()