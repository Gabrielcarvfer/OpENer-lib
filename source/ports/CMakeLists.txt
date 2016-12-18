#######################################
# OpENer ports			              #
#######################################

add_subdirectory( ${OpENer_PLATFORM} )

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platform-specific includes      #
#######################################
opener_platform_support("INCLUDES")

set( PLATFORM_GENERIC_SRC ${OpENer})
