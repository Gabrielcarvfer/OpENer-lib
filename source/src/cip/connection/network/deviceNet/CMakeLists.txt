

set( CIP_NET_DNET_SRC NET_DeviceNetEncapsulation.cpp NET_DeviceNetProtocol.cpp can_link/NET_CanInterface.cpp)
add_library( CIP_NET_DNET STATIC ${CIP_NET_DNET_SRC})

if(WIN32)
	target_link_libraries(CIP_NET_DNET ${WIN32_LIBS})
elseif(LINUX)
	target_link_libraries(CIP_NET_DNET)
endif()