//
// @author Gabriel Ferreira (gabrielcarvfer)
//
#ifndef OPENER_CIPDEVICENETLINK_H_
#define OPENER_CIPDEVICENETLINK_H_

#include "../../../template/CIP_Object.hpp"
#include "../../../ciptypes.hpp"



class NET_DeviceNET_Link;

class NET_DeviceNET_Link : public CIP_Object<NET_DeviceNET_Link>
{
	NET_DeviceNET_Link();
	~NET_DeviceNET_Link();

	CipStatus CipDevicenetLinkInit();
	void ConfigureMacAddress(const CipUsint* mac_address);
	CipStatus GetAttributeSingleDeviceNetInterface(CipMessageRouterRequest* message_router_request, CipMessageRouterResponse* message_router_response);
	CipStatus InstanceServices(int service, CipMessageRouterRequest * msg_router_request, CipMessageRouterResponse* msg_router_response);

	private:
		CipUsint mac_id;
		CipUsint baud_rate;
		CipBool BOI; //bus-off interrupt
		CipUsint BOC; //bus-off counter
		CipBool mac_id_switch;
		CipBool baud_rate_switch;
		CipUsint mac_id_switch_val;
		CipUsint baud_rate_switch_val;
		CipBool quick_connect;

};

#endif /* OPENER_CIPDEVICENETLINK_H_*/
