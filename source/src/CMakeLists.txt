#######################################
# Add subdirectories                  #
#######################################
opener_common_includes()

add_subdirectory( cip_objects )
add_subdirectory( utils )
add_subdirectory( cip )

set( OPENER_SRC typedefs.h trace.h opener_user_conf.h Opener_ExplicitConnection.cpp Opener_IOConnection.cpp  Opener_Interface.cpp) #${CIP_SRC} ${UTILS_SRC} )

add_library( OpENer SHARED ${OPENER_SRC})

if(WIN32)
    target_link_libraries(OpENer OpENer_CLASS OpENer_UTILS OpENer_CONN OpENer_NET OpENer_CIP CIP_NET_ETHIP wsock32 ws2_32)
endif()
