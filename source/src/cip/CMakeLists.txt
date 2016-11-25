#######################################
# CIP library                         #
#######################################
add_subdirectory( class_stack )
add_subdirectory( connection_stack )
add_subdirectory( network_stack )

#######################################
# Add common includes                 #
#######################################
opener_common_includes()

#######################################
# Add platform-specific includes      #
#######################################
opener_platform_support("INCLUDES")

set( CIP_SRC appcontype.cpp CIP_Assembly.cpp ciperror.h CIP_Identity.cpp ciptypes.h CIP_CommonPacket.cpp ../Opener_Interface.cpp ../Opener_Interface.h ../Opener_ExplicitConnection.cpp ../Opener_ExplicitConnection.h ../Opener_IOConnection.cpp ../Opener_IOConnection.h)

add_library( CIP ${CIP_SRC} ${UTILS_SRC} ${CIP_NET_SRC} ${CIP_CLASS_SRC})
