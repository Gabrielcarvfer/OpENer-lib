opENer_common_includes()

set( CIP_CLASS_SRC CIP_Attribute.cpp CIP_Service.cpp CIP_Object.hpp CIP_Object_impl.hpp)

add_library( CIP_CLASS STATIC ${CIP_CLASS_SRC})
