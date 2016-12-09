
add_subdirectory( class_stack )
add_subdirectory( connection_stack )
add_subdirectory( network_stack )

opener_common_includes()

set( CIP_SRC ${CIP_CLASS_SRC} ${CIP_CONN_SRC} ${CIP_NET_SRC} appcontype.cpp CIP_Assembly.cpp CIP_CommonPacket.cpp CIP_Identity.cpp ciptypes.h ciperror.h)
add_library( OpENer_CIP ${CIP_SRC})
target_link_libraries(OpENer_CIP OpENer_CLASS OpENer_CONN OpENer_NET)