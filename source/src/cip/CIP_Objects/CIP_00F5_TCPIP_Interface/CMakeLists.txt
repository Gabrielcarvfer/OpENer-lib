opENer_common_includes()

set( CIP_CLASS_SRC CIP_TCPIP_Interface.cpp)

add_library( CIP_CLASS00F5_TCPIPINTERFACE STATIC ${CIP_CLASS_SRC})

if(WIN32)
    #if MINGW
    if(MINGW)
        target_link_libraries(CIP_CLASS00F5_TCPIPINTERFACE ws2_32 wsock32)
        #if Visual Studio
    else ()
        add_definitions(-DVSTUDIO)
    endif()
elseif(LINUX)
endif()