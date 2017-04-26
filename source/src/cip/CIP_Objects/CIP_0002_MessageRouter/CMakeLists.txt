opENer_common_includes()

set( CIP_CLASS_SRC CIP_MessageRouter.cpp ../../CIP_ElectronicKey.cpp)

add_library( CIP_CLASS0002_MSGROUTER STATIC ${CIP_CLASS_SRC})
target_link_libraries(CIP_CLASS0002_MSGROUTER OpENer_CIP)
