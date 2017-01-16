opENer_common_includes()

add_subdirectory(template)
add_subdirectory(connection)

set( CIP_SRC CIP_Appcontype.cpp CIP_Assembly.cpp CIP_Common.cpp CIP_Analog_Input_Point.cpp ciptypes.hpp ciperror.hpp) #${CIP_CLASS_SRC} ${CIP_CONN_SRC} ${CIP_NET_SRC} )
add_library( OpENer_CIP STATIC ${CIP_SRC})

target_link_libraries(OpENer_CIP OpENer_CLASS OpENer_CONN)