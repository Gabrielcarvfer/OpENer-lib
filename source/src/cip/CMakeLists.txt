opener_common_includes()

add_subdirectory( class_stack )
add_subdirectory( connection_stack )

set( CIP_SRC CIP_Appcontype.cpp CIP_Assembly.cpp CIP_Common.cpp ciptypes.hpp ciperror.hpp) #${CIP_CLASS_SRC} ${CIP_CONN_SRC} ${CIP_NET_SRC} )
add_library( OpENer_CIP STATIC ${CIP_SRC})

target_link_libraries(OpENer_CIP OpENer_CLASS OpENer_CONN)