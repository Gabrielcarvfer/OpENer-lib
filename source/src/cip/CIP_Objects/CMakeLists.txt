opENer_common_includes()

add_subdirectory(template)
add_subdirectory(CIP_0001_Identity)
add_subdirectory(CIP_0002_MessageRouter)
add_subdirectory(CIP_0003_DeviceNET)
add_subdirectory(CIP_0004_Assembly)
add_subdirectory(CIP_0006_ConnectionManager)
add_subdirectory(CIP_000A_AnalogInput)
add_subdirectory(CIP_00F5_TCPIP_Interface)
add_subdirectory(CIP_00F6_EthernetLink)

add_library( CIP_Objects STATIC)

target_link_libraries(
        CIP_Objects
        CIP_CLASS
        CIP_CLASS0001_IDENTITY
        CIP_CLASS0002_MSGROUTER
        CIP_CLASS0003_DEVICENET
        CIP_CLASS0004_ASSEMBLY
        CIP_CLASS0006_CONNECTIONMANAGER
        CIP_CLASS000A_ANALOGINPUT
        CIP_CLASS00F5_TCPIPINTERFACE
        CIP_CLASS00F6_ETHERNETLINK
        )