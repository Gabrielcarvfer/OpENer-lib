opENer_common_includes()

add_subdirectory(network)

set( CIP_CONN_SRC CIP_CommonPacket.cpp CIP_Class3Connection.cpp CIP_IOConnection.cpp )
add_library( OpENer_CONN STATIC ${CIP_CONN_SRC})

target_link_libraries(OpENer_CONN OpENer_NET)
