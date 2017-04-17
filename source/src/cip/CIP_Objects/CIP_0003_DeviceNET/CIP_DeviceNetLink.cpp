//
// Gabriel Ferreira (@gabrielcarvfer)
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
#include "CIP_DeviceNetLink.hpp"
 
//Variables
//CipUdint CIP_DeviceNET_Link::class_id = kCipDeviceNetLinkClassCode;
//std::string CIP_DeviceNET_Link::class_name = "Devicenet Link";
//CipUdint CIP_DeviceNET_Link::get_all_class_attributes_mask = 0xffffffff;
//CipUdint CIP_DeviceNET_Link::get_all_instance_attributes_mask = 0xffffffff;

//Methods

CIP_DeviceNET_Link::~CIP_DeviceNET_Link()
{

}


CipStatus CIP_DeviceNET_Link::Init()
{
    // set attributes to initial values
    //baud_rate = 0;
    //BOI = false;
    //BOC = 0;
    //mac_id_switch = false;
    //baud_rate_switch = false;
    //baud_rate_switch_val = 0;
    //quick_connect = false;
    
    if (number_of_instances == 0)
	{
		CIP_DeviceNET_Link * class_ptr = new CIP_DeviceNET_Link();

        //DeviceNet Object Attribute - Revision
        class_ptr->InsertAttribute(1, kCipUint,  &revision, kGetableSingle);

        //DeviceNet Object Service - GetAttributeSingle
		//class_ptr->InsertService(kGetAttributeSingle,    &GetAttributeSingleDeviceNetInterface, "GetAttributeSingleDeviceNetInterface");
        //service code 0E
		object_Set.emplace(0, class_ptr);

		CIP_DeviceNET_Link * instance_ptr = new CIP_DeviceNET_Link();
		instance_ptr->class_ptr = class_ptr;

        //DeviceNet instance attributes

        //MAC ID (USINT)
		instance_ptr->InsertAttribute(1, kCipUsint, &instance_ptr->mac_id, kSetAndGetAble); // bind attributes to the instance

        //Baud Rate (USINT)
		instance_ptr->InsertAttribute(2, kCipUsint, &instance_ptr->baud_rate, kSetAndGetAble);

        //BOI (BOOL)
		instance_ptr->InsertAttribute(3, kCipBool, &instance_ptr->BOI, kSetAndGetAble);

        //Bus-Off Counter (USINT)
		instance_ptr->InsertAttribute(4, kCipUsint, &instance_ptr->BOC, kSetAndGetAble);

        //Allocation Information (STRUCT)
        //Allocation Choice Byte (BYTE)
        //Master MAC ID (USINT)

        //MAC ID Switch Changed (BOOL)
		instance_ptr->InsertAttribute(6, kCipBool, &instance_ptr->mac_id_switch, kGetableSingleAndAll);

        //Baud Rate Switch Changed (BOOL)
		instance_ptr->InsertAttribute(7, kCipBool, &instance_ptr->baud_rate_switch, kGetableSingleAndAll);

        //Mac ID Switch Value (USINT)
		instance_ptr->InsertAttribute(8, kCipUsint, &instance_ptr->mac_id_switch_val, kGetableSingleAndAll);

        //Baud Rate Switch Value (USINT)
		instance_ptr->InsertAttribute(9, kCipUsint, &instance_ptr->baud_rate_switch_val, kGetableSingleAndAll);

        //Quick Connect (BOOL)
		instance_ptr->InsertAttribute(10, kCipBool, &instance_ptr->quick_connect, kSetable);

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
	return kCipGeneralStatusCodeSuccess;
}

CipStatus CIP_DeviceNET_Link::GetAttributeSingleDeviceNetInterface(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response)
{

    CipStatus status = kCipGeneralStatusCodeSuccess;
    CipByte* message = (CipByte*)&message_router_response->response_data[0];

    status = this->GetAttributeSingle(message_router_request,   message_router_response);

    return status;
}

CipStatus CIP_DeviceNET_Link::InstanceServices(int service, CipMessageRouterRequest_t * msg_router_request, CipMessageRouterResponse_t* msg_router_response)
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