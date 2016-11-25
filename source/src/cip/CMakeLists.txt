
add_subdirectory( class_stack )
add_subdirectory( connection_stack )
add_subdirectory( network_stack )

opener_common_includes()

set( CIP_SRC ${CIP_CLASS_SRC} ${CIP_CONN_SRC} ${CIP_NET_SRC} ciptypes.h ciperror.h CIP_Assembly.cpp CIP_CommonPacket.cpp CIP_Identity.cpp)

add_library( Cip ${CIP_SRC} )
