

set( CIP_CLASS_SRC CIP_ConnectionManager.cpp CIP_ConnectionManager.hpp)

add_library( CIP_CLASS0006_CONNECTIONMANAGER STATIC ${CIP_CLASS_SRC})

target_link_libraries(CIP_CLASS0006_CONNECTIONMANAGER OpENer_CONN)