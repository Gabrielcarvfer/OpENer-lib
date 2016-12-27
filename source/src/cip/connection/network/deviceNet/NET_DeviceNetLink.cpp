//
// Gabriel Ferreira (gabrielcarvfer)
//

//TODO:
//Connection ID in Slaves identify their purpose
//
//	Predefined IDs are:
//
//	ConnID #   |  Utility
//	1          |  references Explicit message conn into server
//	2          |  references Poll I/O conn
//	3          |  references bit-strobe I/O conn
//	4          |  references the Slave changeOfState or Cyclic conn
//	5          |  references multicast poll I/O
//

//Includes
#include <cstring>
#include "NET_DeviceNetLink.h"
 
//Variables
CipUdint NET_DeviceNET_Link::class_id = kCipDeviceNetLinkClassCode;
std::string NET_DeviceNET_Link::class_name = "Devicenet Link";
CipUdint NET_DeviceNET_Link::get_all_class_attributes_mask = 0xffffffff;
CipUdint NET_DeviceNET_Link::get_all_instance_attributes_mask = 0xffffffff;

//Methods
void NET_DeviceNET_Link::ConfigureMacAddress(const CipUsint* mac_address)
{
    memcpy(&mac_id, mac_address,  sizeof(mac_id));
}

NET_DeviceNET_Link::NET_DeviceNET_Link()
{

}

NET_DeviceNET_Link::~NET_DeviceNET_Link()
{

}


CipStatus NET_DeviceNET_Link::CipDevicenetLinkInit()
{
    // set attributes to initial values
    baud_rate = 0;
    BOI = false;
    BOC = 0;
    mac_id_switch = false;
    baud_rate_switch = false;
    baud_rate_switch_val = 0;
    quick_connect = false;
    
    if (object_Set.size() == 0)
	{
		NET_DeviceNET_Link * class_ptr = new NET_DeviceNET_Link();

        //DeviceNet Object Attribute - Revision
        class_ptr->InsertAttribute(1, kCipUint,  &revision, kGetableSingle);

        //DeviceNet Object Service - GetAttributeSingle
		//class_ptr->InsertService(kGetAttributeSingle,    &GetAttributeSingleDeviceNetInterface, "GetAttributeSingleDeviceNetInterface");
        //service code 0E
		object_Set.emplace(0, class_ptr);

		NET_DeviceNET_Link * instance_ptr = new NET_DeviceNET_Link();
		instance_ptr->class_ptr = class_ptr;

        //DeviceNet instance attributes

        //MAC ID (USINT)
		instance_ptr->InsertAttribute(1, kCipUsint, &mac_id, kSetAndGetAble); // bind attributes to the instance

        //Baud Rate (USINT)
		instance_ptr->InsertAttribute(2, kCipUsint, &baud_rate, kSetAndGetAble);

        //BOI (BOOL)
		instance_ptr->InsertAttribute(3, kCipBool, &BOI, kSetAndGetAble);

        //Bus-Off Counter (USINT)
		instance_ptr->InsertAttribute(4, kCipUsint, &BOC, kSetAndGetAble);

        //Allocation Information (STRUCT)
        //Allocation Choice Byte (BYTE)
        //Master MAC ID (USINT)

        //MAC ID Switch Changed (BOOL)
		instance_ptr->InsertAttribute(6, kCipBool, &mac_id_switch, kGetableSingleAndAll);

        //Baud Rate Switch Changed (BOOL)
		instance_ptr->InsertAttribute(7, kCipBool, &baud_rate_switch, kGetableSingleAndAll);

        //Mac ID Switch Value (USINT)
		instance_ptr->InsertAttribute(8, kCipUsint, &mac_id_switch_val, kGetableSingleAndAll);

        //Baud Rate Switch Value (USINT)
		instance_ptr->InsertAttribute(9, kCipUsint, &baud_rate_switch_val, kGetableSingleAndAll);

        //Quick Connect (BOOL)
		instance_ptr->InsertAttribute(10, kCipBool, &quick_connect, kSetable);

        //Safety Network Number
        //InsertAttribute(3, kCipBool,
        //                &physical_address, kGetableSingleAndAll);

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
		object_Set.emplace(instance_ptr->id, instance_ptr);

    }
	else 
	{
        return kCipStatusError;
    }

    return kCipStatusOk;
}

CipStatus NET_DeviceNET_Link::GetAttributeSingleDeviceNetInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response)
{

    CipStatus status = kCipStatusOkSend;
    CipByte* message = message_router_response->data;

    status = this->GetAttributeSingle(message_router_request,   message_router_response);

    return status;
}

CipStatus NET_DeviceNET_Link::InstanceServices(int service, CipMessageRouterRequest * msg_router_request, CipMessageRouterResponse* msg_router_response)
{
	switch (service)
	{
		case kGetAttributeSingle:
			return this->GetAttributeSingleDeviceNetInterface(msg_router_request, msg_router_response);
			break;
		default:
			return kCipStatusError;
			break;
	}
}