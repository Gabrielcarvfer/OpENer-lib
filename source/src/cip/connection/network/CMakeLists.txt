opENer_common_includes()

set( CIP_NET_SRC
		NET_Encapsulation.cpp
		NET_Connection.cpp
		NET_NetworkHandler.cpp
		NET_Endianconv.cpp
		./ethIP/NET_EthIP_Encap.cpp
		./ethIP/eip_endianconv.cpp
		./ethIP/NET_EthIP_Includes.h
		../CIP_CommonPacket.cpp)
		#${CIP_NET_DNET_SRC})

add_library( OpENer_NET STATIC ${CIP_NET_SRC})

if(WIN32)
	target_link_libraries(OpENer_NET
			OpENerLib
			OpENer_CONN
			CIP_CLASS00F6_ETHERNETLINK
			CIP_CLASS0001_IDENTITY
			ws2_32.lib
			wsock32)# CIP_NET_DNET
elseif(LINUX)
	target_link_libraries(OpENer_NET
			OpENerLib
			OpENer_CONN
			CIP_CLASS00F6_ETHERNETLINK
			CIP_CLASS0001_IDENTITY)#CIP_NET_ETHIP CIP_NET_DNET
endif()