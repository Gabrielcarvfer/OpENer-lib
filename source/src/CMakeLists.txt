#######################################
# Add subdirectories                  #
#######################################

add_subdirectory( cip )
add_subdirectory( cip_objects )
add_subdirectory( ports )
add_subdirectory( utils )

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platfrom specific things        #
#######################################

opener_platform_support( "INCLUDES" )
set( OPENER_SRC ${CIP_SRC} ${PLATFORM_GENERIC_SRC} typedefs.h trace.h opener_user_conf.h Opener_ExplicitConnection.cpp Opener_IOConnection.cpp  Opener_Interface.cpp)

