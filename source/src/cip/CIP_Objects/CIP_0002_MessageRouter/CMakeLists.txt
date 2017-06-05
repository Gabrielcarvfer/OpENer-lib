opENer_common_includes()

set( CIP_CLASS0002_SRC
        CIP_MessageRouter.cpp
        CIP_MessageRouter.hpp
        ../CIP_Object.hpp
        ../CIP_ClassStack.cpp
        ../../CIP_Common.cpp
        ../../CIP_ElectronicKey.cpp
        ../../CIP_AppConnType.cpp
        )

add_library( CIP_CLASS0002_MSGROUTER STATIC ${CIP_CLASS0002_SRC})
target_link_libraries(CIP_CLASS0002_MSGROUTER OpENer_CIP)
