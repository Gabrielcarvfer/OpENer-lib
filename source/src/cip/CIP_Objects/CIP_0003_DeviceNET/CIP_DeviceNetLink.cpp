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
        class_ptr->classAttrInfo.emplace(1, CipAttrInfo_t{kCipUint,  sizeof(CipUint), kGetableSingle, "Revision"});

        //DeviceNet Object Service - GetAttributeSingle
		//class_ptr->InsertService(kGetAttributeSingle,    &GetAttributeSingleDeviceNetInterface, "GetAttributeSingleDeviceNetInterface");
        //service code 0E
		object_Set.emplace(0, class_ptr);

        //DeviceNet instance attributes
		class_ptr->instAttrInfo.emplace( 1, CipAttrInfo_t{kCipUsint, sizeof(CipUsint), kSetAndGetAble, "mac_id"   }); // bind attributes to the instance
		class_ptr->instAttrInfo.emplace( 2, CipAttrInfo_t{kCipUsint, sizeof(CipUsint), kSetAndGetAble, "baud_rate"});
		class_ptr->instAttrInfo.emplace( 3, CipAttrInfo_t{kCipBool , sizeof(CipBool ), kSetAndGetAble, "BOI"      });
		class_ptr->instAttrInfo.emplace( 4, CipAttrInfo_t{kCipUsint, sizeof(CipUsint), kSetAndGetAble, "BOC"      });

        //Allocation Information (STRUCT)
        //Allocation Choice Byte (BYTE)
        //Master MAC ID (USINT)
		class_ptr->instAttrInfo.emplace( 6, CipAttrInfo_t{kCipBool , sizeof(CipBool ), kGetableSingleAndAll, "mac_id_switch"       });
		class_ptr->instAttrInfo.emplace( 7, CipAttrInfo_t{kCipBool , sizeof(CipBool ), kGetableSingleAndAll, "baud_rate_switch"    });
		class_ptr->instAttrInfo.emplace( 8, CipAttrInfo_t{kCipUsint, sizeof(CipUsint), kGetableSingleAndAll, "mac_id_switch_val"   });
		class_ptr->instAttrInfo.emplace( 9, CipAttrInfo_t{kCipUsint, sizeof(CipUsint), kGetableSingleAndAll, "baud_rate_switch_val"});
		class_ptr->instAttrInfo.emplace(10, CipAttrInfo_t{kCipBool , sizeof(CipBool ), kSetable            , "quick_connect"       });

        //Safety Network Number
        //instAttrInfo.emplace(3, kCipBool,
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
		object_Set.emplace(class_ptr->id, class_ptr);
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

CipStatus CIP_DeviceNET_Link::Shut()
{

}

CipStatus CIP_DeviceNET_Link::Create()
{

}

CipStatus CIP_DeviceNET_Link::Delete()
{

}

void * CIP_DeviceNET_Link::retrieveAttribute(CipUsint attributeNumber)
{
    if (this->id == 0)
    {
        switch(attributeNumber)
        {
            case 1: return &this->revision;
            default: return nullptr;
        }
    }
    else
    {
        switch(attributeNumber)
        {
            case  1: return &this->mac_id;
            case  2: return &this->serial;
            case  3: return &this->baud_rate;
            case  4: return &this->vendor_id;
            case  5: return &this->BOI; //bus-off interrup
            case  6: return &this->BOC; //bus-off counter
            case  7: return &this->mac_id_switch;
            case  8: return &this->baud_rate_switch;
            case  9: return &this->mac_id_switch_val;
            case 10: return &this->baud_rate_switch_val;
            case 11: return &this->quick_connect;
            case 12: return &this->physical_port;
            default: return nullptr;
        }
    }
}

CipStatus CIP_DeviceNET_Link::retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp)
{

}