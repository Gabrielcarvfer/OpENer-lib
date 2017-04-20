opENer_common_includes()

set( CIP_TEST_SRC ./TEST_Cip_Connection.cpp)

add_executable( TEST_CIP_Connection ${CIP_TEST_SRC})
target_link_libraries (TEST_CIP_Connection CIP_CLASS0005_CONNECTION)

add_test(UNITTEST_CIP_Connection TEST_CIP_Connection )