opENer_common_includes()

set( CIP_TEST_SRC TEST_Cip_Template.cpp ../CIP_Object_template.hpp)

add_executable( TEST_CIP_template ${CIP_TEST_SRC})

add_test(UNITTEST_CIP_template TEST_CIP_template )