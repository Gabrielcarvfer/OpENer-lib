//
// @author Gabriel Ferreira (gabrielcarvfer)
//
#ifndef OPENER_CIPDEVICENETLINK_H_
#define OPENER_CIPDEVICENETLINK_H_

#include "../../../template/CIP_Object.hpp"
#include "../../../ciptypes.hpp"
#include "can_link/NET_CanInterface.h"



class NET_DeviceNET_Link;

class NET_DeviceNET_Link : public CIP_Object<NET_DeviceNET_Link>
{
	NET_DeviceNET_Link
	(
		 CipUdint serial, CipUsint baud_rate, CipUint vendor_id,
		 CipBool BOI, CipUsint BOC, CipBool mac_id_switch, CipUsint baud_rate_switch,
		 CipUsint mac_id_switch_val, CipUsint baud_rate_switch_val, CipBool quick_connect,
		 CipByte physical_port, NET_CanInterface * associated_can, CipUsint mac_id = 0xFF
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
	NET_DeviceNET_Link() { mac_id = 0xFF; };
	~NET_DeviceNET_Link();

	CipStatus CipDevicenetLinkInit();
	CipStatus GetAttributeSingleDeviceNetInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);
	CipStatus InstanceServices(int service, CipMessageRouterRequest * msg_router_request, CipMessageRouterResponse* msg_router_response);

	private:
		CipUsint mac_id;
		CipUdint serial;
		CipUsint baud_rate;
		CipUint vendor_id;
		CipBool BOI; //bus-off interrupt
		CipUsint BOC; //bus-off counter
		CipBool mac_id_switch;
		CipBool baud_rate_switch;
		CipUsint mac_id_switch_val;
		CipUsint baud_rate_switch_val;
		CipBool quick_connect;
		CipByte physical_port;
		NET_CanInterface * associated_can;

};

#endif /* OPENER_CIPDEVICENETLINK_H_*/
