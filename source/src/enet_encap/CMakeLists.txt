#######################################
# Ethernet encapsulation library      #
#######################################

set( ENET_ENCAP_SRC eip_encap.c eip_endianconv.c ${UTILS_SRC})

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platform-specific includes      #
#######################################
opener_platform_support("INCLUDES")

add_library( ENET_ENCAP ${ENET_ENCAP_SRC} )

target_link_libraries(ENET_ENCAP Utils)
