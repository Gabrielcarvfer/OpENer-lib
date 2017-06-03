//
// Created by Gabriel Ferreira (@gabrielcarvfer)
//
#ifndef OPENER_CIPDEVICENETLINK_H_
#define OPENER_CIPDEVICENETLINK_H_

#include "cip/CIP_Objects/template/CIP_Object_template.hpp"
#include "../../ciptypes.hpp"
#include "NET_DeviceNetProtocol.hpp"



class CIP_DeviceNET_Link : public CIP_Object<CIP_DeviceNET_Link>
{
public:
	CIP_DeviceNET_Link
	(
		 CipUdint serial, CipUsint baud_rate, CipUint vendor_id,
		 CipBool BOI, CipUsint BOC, CipBool mac_id_switch, CipUsint baud_rate_switch,
		 CipUsint mac_id_switch_val, CipUsint baud_rate_switch_val, CipBool quick_connect,
		 CipByte physical_port, NET_DeviceNetProtocol * associated_devicenet, CipUsint mac_id = 0xFF
	) 
	{
		mac_id = mac_id;
		serial = serial;
		baud_rate = baud_rate;
		vendor_id = vendor_id;
		BOI = BOI; //bus-off interrupt
		BOC = BOC; //bus-off counter
		mac_id_switch = mac_id_switch;
		baud_rate_switch = baud_rate_switch;
		mac_id_switch_val = mac_id_switch_val;
		baud_rate_switch_val = baud_rate_switch_val;
		quick_connect = quick_connect;
		physical_port = physical_port;
		associated_can;
	};
	CIP_DeviceNET_Link() { mac_id = 0xFF; };
	~CIP_DeviceNET_Link();

	static CipStatus Init();
	static CipStatus Shut();
	static CipStatus Create();
	static CipStatus Delete();
	CipStatus GetAttributeSingleDeviceNetInterface(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response);

	private:
		CipUsint mac_id;
		CipUdint serial;
		CipUsint baud_rate;
		CipUint  vendor_id;
		CipBool  BOI; //bus-off interrupt
		CipUsint BOC; //bus-off counter
		CipBool  mac_id_switch;
		CipBool  baud_rate_switch;
		CipUsint mac_id_switch_val;
		CipUsint baud_rate_switch_val;
		CipBool  quick_connect;
		CipByte  physical_port;
		NET_DeviceNetProtocol * associated_can;

    void * retrieveAttribute(CipUsint attributeNumber);
    CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);

};

#endif /* OPENER_CIPDEVICENETLINK_H_*/
