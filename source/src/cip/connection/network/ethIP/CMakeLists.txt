opENer_common_includes()

set( CIP_NET_ETHIP_SRC NET_EthIP_Encap.cpp NET_EthIP_Includes.h eip_endianconv.cpp)
add_library( CIP_NET_ETHIP STATIC ${CIP_NET_ETHIP_SRC})

if(__WIN32__)
	#if MINGW
	if(MINGW)
		target_link_libraries(CIP_NET_ETHIP OpENer_CONN ws2_32 wsock32)
	#if Visual Studio
	else ()
		add_definitions(-DVSTUDIO)
		target_link_libraries(CIP_NET_ETHIP)
	endif()
elseif(LINUX)
	target_link_libraries(CIP_NET_ETHIP)
endif()