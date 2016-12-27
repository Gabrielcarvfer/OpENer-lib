#######################################
# Add subdirectories                  #
#######################################
opENer_common_includes()

add_subdirectory( cip_objects )
add_subdirectory( utils )
add_subdirectory( cip )

set( OPENER_SRC OpENer_ExplicitConnection.cpp OpENer_IOConnection.cpp OpENer_Interface.cpp)

add_library( OpENer STATIC ${OPENER_SRC})# ${CIP_SRC} ${UTILS_SRC} )

target_link_libraries(OpENer OpENer_CIP OpENer_UTILS)

if(WIN32)
  if(MINGW)
	target_link_libraries(OpENer OpENer_CIP OpENer_UTILS winmm)
  else ()
	add_definitions(-DVSTUDIO)
	target_link_libraries(OpENer OpENer_CIP OpENer_UTILS)
  endif()
endif()
