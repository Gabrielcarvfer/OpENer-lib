opENer_common_includes()

set( CIP_CLASS_SRC CIP_Connection.cpp CIP_Connection_LinkProducer.cpp CIP_Connection_LinkConsumer.cpp)

add_library( CIP_CLASS0005_CONNECTION STATIC ${CIP_CLASS_SRC})

target_link_libraries(CIP_CLASS0005_CONNECTION OpENer_CONN)