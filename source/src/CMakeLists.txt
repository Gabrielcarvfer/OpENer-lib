#######################################
# Add subdirectories                  #
#######################################
opener_common_includes()

add_subdirectory( cip_objects )
add_subdirectory( utils )
add_subdirectory( cip )

set( OPENER_SRC typedefs.hpp trace.hpp opener_user_conf.hpp Opener_ExplicitConnection.cpp Opener_IOConnection.cpp  Opener_Interface.cpp)

add_library( OpENer STATIC ${OPENER_SRC})# ${CIP_SRC} ${UTILS_SRC} )

target_link_libraries(OpENer OpENer_CIP OpENer_UTILS)

