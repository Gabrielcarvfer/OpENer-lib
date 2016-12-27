#include "NET_DeviceNetProtocol.h"
#include "can_link/NET_CanInterface.h"
//
//	Get datagram group from CAN identifier
//
int dnet_identifier_group_check(uint32_t* can_id)
{

    //Check if identifier matches with Message Group 1
    if (*can_id <= 0x3ff) {
        return 1;
    }

    //Check if identifier matches with Message Group 2
    if (*can_id <= 0x5ff) {
        return 2;
    }

    //Check if identifier matches with Message Group 3
    if (*can_id <= 0x7bf) {
        return 3;
    }

    //Check if identifier matches with Message Group 4
    if (*can_id <= 0x7cf) {
        return 4;
    }

    //If failed every test, invalid identifier
    return -1;
}

//
//	Get info from CAN identifier based on Group ID
//
int dnet_identifier_group_match(struct can_frame* frame_rd)
{
    kDeviceNetMessageId msg_id;
    int mac;

    //Get message ID and MAC source/dest depending of message group (11bit ID only)
    switch (dnet_identifier_group_check(&(frame_rd->can_id)))
	{
		case (1):
			msg_id = kDeviceNetMessageId((frame_rd->can_id & 0x00003C0) >> 6);
			mac = (frame_rd->can_id & 0x0000003F);
			break;
		case (2):
			msg_id = kDeviceNetMessageId((frame_rd->can_id & 0x00000007));
			mac = (frame_rd->can_id & 0x000001F8) >> 3;
			break;
		case (3):
			msg_id = kDeviceNetMessageId((frame_rd->can_id & 0x00001C0) >> 6);
			mac = (frame_rd->can_id & 0x0000003F);
			break;
		case (4):
			msg_id = kDeviceNetMessageId((frame_rd->can_id & 0x0000002F));
		default:
			return -1;
    }

    //Big switch for each message type
    switch (msg_id) 
	{
		case (slv_mcast_poll_res):

			break;
		case (slv_st8chng_or_cyc):
			break;
		case (slv_bit_strobe_res):
			break;
		case (slv_poll_or_cyc_ack):
			break;
		case (mstr_bit_strobe_cmd):
			break;
		case (mstr_mcast_poll_cmd):
			break;
		case (mstr_st8chng_or_cyc_ack):
			break;
		case (slv_explicit_or_unconnected_res):
			break;
		case (mstr_explicit_req):
			break;
		case (mstr_poll_or_st8chng_or_cyc_req):
			break;
		case (g2only_unconnected_explicit_req):
			break;
		case (duplicate_macId_check):
			break;
		//Unknown ID
		default:
			//printf("fodeu %d\n", msg_id);
			return -1;
    }
    //printf("dnet_identifier -> msg_id = %d source_mac = %d\n", msg_id, mac);
	return 0;
}