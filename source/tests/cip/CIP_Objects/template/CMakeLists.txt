opENer_common_includes()

set( CIP_TEST_SRC ./TEST_Cip_Template.cpp ../../../../src/cip/CIP_Objects/template/CIP_Object.hpp ../../../../src/cip/CIP_Objects/template/CIP_Attribute.cpp ../../../../src/cip/CIP_Objects/template/CIP_Service.cpp)

add_executable( TEST_CIP_template ${CIP_TEST_SRC})

add_test(UNITTEST_CIP_template TEST_CIP_template )