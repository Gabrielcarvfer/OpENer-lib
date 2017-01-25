opENer_common_includes()

add_subdirectory(CIP_Objects)
add_subdirectory(connection)

set( CIP_SRC CIP_Appcontype.cpp CIP_Common.cpp ciptypes.hpp ciperror.hpp)
add_library(OpENer_CIP STATIC ${CIP_SRC})
target_link_libraries(OpENer_CIP OpENer_CONN CIP_Objects )