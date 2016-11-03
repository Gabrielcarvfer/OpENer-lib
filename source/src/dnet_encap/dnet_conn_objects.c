//Connection ID in Slaves identify their purpose
/*
	Predefined IDs are:

	ConnID #   |  Utility
	1          |  references Explicit message conn into server
	2          |  references Poll I/O conn
	3          |  references bit-strobe I/O conn
	4          |  references the Slave changeOfState or Cyclic conn
	5          |  references multicast poll I/O
*/

typedef struct {
    kCipUsint mac_id;
    kCipUsint baud_rate;
    kCipBool BOI; //bus-off interrupt
    kCipUsint BOC; //bus-off counter
    kCipBool mac_id_switch;
    kCipBool baud_rate_switch;
    kCipUsint mac_id_switch_val;
    kCipUsint baud_rate_switch_val;
    kCipBool quick_connect;
} CipDevicenetSlaveObject;

/* global private variables */
CipDevicenetSlaveObject g_devicenet_slave;

CipStatus CipDevicenetSlaveInit()
{
    CIPClass* devicenet_slave_class = 0;
    CipInstance* devicenet_slave_instance;

    /* set attributes to initial values */
    g_devicenet_slave.baud_rate = 0;
    g_devicenet_slave.BOI = false;
    g_devicenet_slave.BOC = 0;
    g_devicenet_slave.mac_id_switch = false;
    g_devicenet_slave.baud_rate_switch = false;
    g_devicenet_slave.baud_rate_switch_val = 0;
    g_devicenet_slave.quick_connect = false;
    g_devicenet_slave.interface_flags = 0xF; /* successful speed and duplex neg, full duplex active link, TODO in future it should be checked if link is active */

    if ((devicenet_slave_class = CreateCIPClass(CIP_DEVICENETSLAVE_CLASS_CODE,
             1, /* # class attributes*/
             0xffffffff, /* class getAttributeAll mask*/
             1, /* # class services*/
             9, /* # instance attributes*/
             0xffffffff, /* instance getAttributeAll mask*/
             0, /* # instance services*/
             1, /* # instances*/
             "Devicenet Slave Object", 1))
        != 0) {

        //DeviceNet Object Attribute - Revision
        InsertAttribute(devicenet_link_class, 1, KCipUint,
            &devicenet_link_class.revision, kGetAttributeSingle);

        //DeviceNet Object Service - GetAttributeSingle
        InsertService(devicenet_link_class, kGetAttributeSingle,
            &GetAttributeSingleDeviceNetInterface,
            "GetAttributeSingleDeviceNetInterface");
        //service code 0E

        //DeviceNet instance attributes
        devicenet_link_instance = GetCipInstance(devicenet_link_class, 1);

        //MAC ID (USINT)
        InsertAttribute(devicenet_link_instance, 1, kCipUsint,
            &g_devicenet_link.mac_id, kSetAndGetAble); /* bind attributes to the instance*/

        //Baud Rate (USINT)
        InsertAttribute(devicenet_link_instance, 2, kCipUsint,
            &g_devicenet_link.baud_rate, kSetAndGetAble);

        //BOI (BOOL)
        InsertAttribute(devicenet_link_instance, 3, kCipBool,
            &g_devicenet_link.BOI, kSetAndGetAble);

        //Bus-Off Counter (USINT)
        InsertAttribute(devicenet_link_instance, 4, kCipUsint,
            &g_devicenet_link.BOC, kSetAndGetAble);

        //Allocation Information (STRUCT)
        //Allocation Choice Byte (BYTE)
        //Master MAC ID (USINT)

        //MAC ID Switch Changed (BOOL)
        InsertAttribute(devicenet_link_instance, 6, kCipBool,
            &g_devicenet_link.mac_id_switch, kGetableSingleAndAll);

        //Baud Rate Switch Changed (BOOL)
        InsertAttribute(devicenet_link_instance, 7, kCipBool,
            &g_devicenet_link.baud_rate_switch, kGetableSingleAndAll);

        //Mac ID Switch Value (USINT)
        InsertAttribute(devicenet_link_instance, 8, kCipUsint,
            &g_devicenet_link.mac_id_switch_val, kGetableSingleAndAll);

        //Baud Rate Switch Value (USINT)
        InsertAttribute(devicenet_link_instance, 9, kCipUsint,
            &g_devicenet_link.baud_rate_switch_val, kGetableSingleAndAll);

        //Quick Connect (BOOL)
        InsertAttribute(devicenet_link_instance, 10, kCipBool,
            &g_devicenet_link.quick_connect, kSetable);

        //Safety Network Number
        //InsertAttribute(devicenet_link_instance, 3, kCipBool,
        //                &g_devicenet_link.physical_address, kGetableSingleAndAll);

        //Diagnostic Counters (STRUCT)
        //Diagnostic Counters Descriptor (WORD)
        //Arbitration Loss Count (UINT)
        //Overload Count (UINT)
        //Bit Error Count (UINT)
        //Stuff error count (UINT)
        //Ack error count (UINT)
        //Form error count (UINT)
        //CRC error count (UINT)
        //Rx message loss count (UINT)
        //Warning error count (UINT)
        //Rx error count (UINT)
        //Tx error count (UINT)
        //Reserved (5xUINT)

        //Active Node Table (ARRAY'o'bool)

    } else {
        return kDnetStatusError;
    }

    return kDnetStatusOk;
}

DnetStatus GetAttributeSingleDeviceNetInterface(
    CipInstance* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{

    DnetStatus status = kDnetStatusOkSend;
    kCipByte* message = message_router_response->data;

    status = GetAttributeSingle(instance, message_router_request,
        message_router_response);

    return status;
}