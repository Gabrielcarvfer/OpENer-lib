opENer_common_includes()

set( CIP_CLASS_SRC CIP_TCPIP_Interface.cpp)

add_library( CIP_CLASS00F5_TCPIPINTERFACE STATIC ${CIP_CLASS_SRC})

if(WIN32)
    target_link_libraries(CIP_CLASS00F5_TCPIPINTERFACE OpENer_CONN OpENer_NET CIP_Objects ws2_32.lib wsock32)#CIP_NET_DNET
elseif(LINUX)
    target_link_libraries(CIP_CLASS00F5_TCPIPINTERFACE OpENer_CONN OpENer_NET CIP_Objects)#CIP_NET_DNET
endif()