set(CIP_TEMPLATE_SRC
        CIP_Object_base.hpp
        CIP_Object_base.cpp
        CIP_Object_template.hpp
        CIP_Object_template_impl.hpp
        )

add_library(CIP_Template STATIC ${CIP_TEMPLATE_SRC})

build_tests()