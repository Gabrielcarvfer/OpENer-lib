opener_common_includes()

set( CIP_CONN_SRC CIP_Common.cpp CIP_Connection.cpp CIP_MessageRouter.cpp CIP_Class3Connection.cpp CIP_IOConnection.cpp)
add_library( OpENer_CONN ${CIP_CONN_SRC})
target_link_libraries(OpENer_CONN OpENer_CLASS OpENer_UTILS OpENer_NET OpENer_CIP)
