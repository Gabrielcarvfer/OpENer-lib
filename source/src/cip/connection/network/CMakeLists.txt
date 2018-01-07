

set( CIP_NET_SRC
		NET_Encapsulation.cpp
		NET_Connection.cpp
		NET_NetworkHandler.cpp
		NET_Endianconv.cpp
		./ethIP/NET_EthIP_Encap.cpp
		./ethIP/eip_endianconv.cpp
        ethIP/NET_EthIP_Includes.hpp
		../CIP_CommonPacket.cpp)
		#${CIP_NET_DNET_SRC})

add_library( OpENer_NET STATIC ${CIP_NET_SRC})


target_link_libraries(OpENer_NET
		OpENerLib
		OpENer_CONN
		CIP_CLASS00F6_ETHERNETLINK
		CIP_CLASS0001_IDENTITY
		${WIN32_LIBS})# CIP_NET_DNET