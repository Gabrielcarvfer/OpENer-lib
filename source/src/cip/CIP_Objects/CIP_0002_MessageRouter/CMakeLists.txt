

set( CIP_CLASS0002_SRC
        CIP_MessageRouter.cpp
        CIP_MessageRouter.hpp
        ../CIP_Object.hpp
        ../CIP_ClassStack.hpp
        ../../CIP_Common.hpp
        ../../CIP_ElectronicKey.hpp
        ../../CIP_AppConnType.hpp
        )

add_library( CIP_CLASS0002_MSGROUTER STATIC ${CIP_CLASS0002_SRC})
target_link_libraries(CIP_CLASS0002_MSGROUTER OpENerLib CIP_Objects CIP_Template )

build_tests()