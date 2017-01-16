
opENer_common_includes()

set( CIP_NET_DNET_SRC NET_DeviceNetEncapsulation.cpp NET_DeviceNetLink.cpp NET_DeviceNetProtocol.cpp can_link/NET_CanInterface.cpp)
add_library( CIP_NET_DNET STATIC ${CIP_NET_DNET_SRC})

if(WIN32)
	#if MINGW
	if(MINGW)
		target_link_libraries(CIP_NET_DNET ws2_32 wsock32)
	#if Visual Studio
	else ()
		add_definitions(-DVSTUDIO)
		target_link_libraries(CIP_NET_DNET)
	endif()
elseif(LINUX)
	target_link_libraries(CIP_NET_DNET)
endif()