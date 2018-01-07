#######################################
# Add subdirectories                  #
#######################################


add_subdirectory( utils )
add_subdirectory( cip )

set( OPENER_SRC OpENer_ExplicitConnection.cpp OpENer_IOConnection.cpp OpENer_Interface.cpp opener_user_conf.hpp trace.hpp typedefs.hpp)

add_library( OpENerLib STATIC ${OPENER_SRC} )

target_link_libraries(OpENerLib
        CIP_CLASS0002_MSGROUTER
        OpENer_CIP
        OpENer_UTILS
        ${WIN32_LIBS})


install (TARGETS OpENerLib
		ARCHIVE DESTINATION lib/static)