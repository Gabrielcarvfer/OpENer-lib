#include <fcntl.h>
#include <linux/can.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include <linux/can/raw.h>

enum dnet_object_error_codes {
    //When Allocate_master/Slave_conn_set is received
    //but Slave already allocated connection to another master
    connection_set_allocation_conflict = 1,

    //Allocate/Release_master/Slave_conn_set request is received
    //but 1) slave dont support specified choice
    //    2) slave asked already allocated/released
    //    3) choice contained all zeros (invalid) or didnt contain explicit message allocation choice
    invalid_choice_parameter = 2,

    //A group2 only server (UCMM incapable) received a message that was not allocate or release
    invalid_g2_message = 3,

    //Resource required	to use groups 1 and 2 is unavailable
    resource_unavailable = 4
};

enum dnet_message_ids {
    /*
		Abreviations meaning
		Prefixes				Infixes						Sufixes        
		slv_ = slave			_cyc_ = cyclic				_res = response       
		mstr_ = master  		_st8chng_ = state change	_req = request        
		g2only_ = only G2   	_mcast_ = multicast			_cmd = command        
															_ack = acknowledge 
	*/

    //Group 1
    slv_mcast_poll_res = 0x1100,
    slv_st8chng_or_cyc = 0x1101,
    slv_bit_strobe_res = 0x1110,
    slv_poll_or_cyc_ack = 0x1111,

    //Group 2:
    mstr_bit_strobe_cmd = 0x000,
    mstr_mcast_poll_cmd = 0x001,
    mstr_st8chng_or_cyc_ack = 0x010,
    slv_explicit_or_unconnected_res = 0x011,
    mstr_explicit_req = 0x100,
    mstr_poll_or_st8chng_or_cyc_req = 0x101,
    g2only_unconnected_explicit_req = 0x110,
    // As in Vol 3, Chapter 2, section 2-3.2
    duplicate_macId_check = 0x111,

    //Group 3

    //Group 4
};

int dnet_identifier_group_check(__u32* can_id);
int dnet_identifier_group_match(struct can_frame* frame_rd);