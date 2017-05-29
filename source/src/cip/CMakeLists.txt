opENer_common_includes()

add_subdirectory(CIP_Objects)
add_subdirectory(connection)

set( CIP_SRC CIP_AppConnType.cpp CIP_Common.cpp ciptypes.hpp ciperror.hpp CIP_ElectronicKey.cpp CIP_Segment.cpp ./connection/network/NET_Encapsulation.hpp)
add_library(OpENer_CIP STATIC ${CIP_SRC})
target_link_libraries(OpENer_CIP OpENer_CONN CIP_Objects )