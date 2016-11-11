/*******************************************************************************
 * Gabriel Ferreira (gabrielcarvfer)
 ******************************************************************************/
#include <string.h>

#include "cipdevicenetlink.h"

#include "src/cip/connection_stack/cipcommon.h"
#include "ciperror.h"
#include "src/cip/connection_stack/cipmessagerouter.h"
#include "endianconv.h"
#include "opener_api.h"
#include "typedefs.h"

typedef struct {
    CipUsint mac_id;
    CipUsint baud_rate;
    CipBool BOI; //bus-off interrupt
    CipUsint BOC; //bus-off counter
    CipBool mac_id_switch;
    CipBool baud_rate_switch;
    CipUsint mac_id_switch_val;
    CipUsint baud_rate_switch_val;
    CipBool quick_connect;
} CipDevicenetLinkObject;

/* global private variables */
CipDevicenetLinkObject g_devicenet_link;

void ConfigureMacAddress(const CipUsint* mac_address)
{
    memcpy(&g_devicenet_link.mac_id, mac_address,
        sizeof(g_devicenet_link.mac_id));
}

class CIPDevicenetNetLink : public CIPClass
{

}

CipStatus CIPDeviceNetLink::CipDevicenetLinkInit()
{
    this.attributes.insert(0, devicenet_link_class.revision);

}
CipStatus CipDevicenetLinkInit()
{
    CIPClass* devicenet_link_class;
    CIPClass* devicenet_link_instance;

    /* set attributes to initial values */
    g_devicenet_link.baud_rate = 0;
    g_devicenet_link.BOI = false;
    g_devicenet_link.BOC = 0;
    g_devicenet_link.mac_id_switch = false;
    g_devicenet_link.baud_rate_switch = false;
    g_devicenet_link.baud_rate_switch_val = 0;
    g_devicenet_link.quick_connect = false;
    g_devicenet_link.interface_flags = 0xF; /* successful speed and duplex neg, full duplex active link, TODO in future it should be checked if link is active */

    if ((devicenet_link_class = CreateCIPClass(kCipDeviceNetLinkClassCode, 1, /* # class attributes*/
             0xffffffff, /* class getAttributeAll mask*/
             1, /* # class services*/
             9, /* # instance attributes*/
             0xffffffff, /* instance getAttributeAll mask*/
             0, /* # instance services*/
             1, /* # instances*/
             "Devicenet Link", 1))
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
        devicenet_link_instance = GetCIPClass(devicenet_link_class, 1);

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
    CIPClass* instance, CipMessageRouterRequest* message_router_request,
    CipMessageRouterResponse* message_router_response)
{

    DnetStatus status = kDnetStatusOkSend;
    kCipByte* message = message_router_response->data;

    status = GetAttributeSingle(instance, message_router_request,
        message_router_response);

    return status;
}