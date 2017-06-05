#######################################
# Add subdirectories                  #
#######################################
opENer_common_includes()

add_subdirectory( utils )
add_subdirectory( cip )

set( OPENER_SRC OpENer_ExplicitConnection.cpp OpENer_IOConnection.cpp OpENer_Interface.cpp opener_user_conf.hpp trace.hpp typedefs.hpp)

add_library( OpENerLib STATIC ${OPENER_SRC} )



if(__WIN32__)
  if(MINGW)
	target_link_libraries(OpENerLib OpENer_CIP OpENer_UTILS winmm )
  else ()
	add_definitions(-DVSTUDIO)
	target_link_libraries(OpENerLib OpENer_CIP OpENer_UTILS )
  endif()
else()
	target_link_libraries(OpENerLib OpENer_CIP OpENer_UTILS )
endif()

install (TARGETS OpENerLib
		ARCHIVE DESTINATION lib/static)