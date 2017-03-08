//
// Created by Gabriel Ferreira (@gabrielcarvfer)
//
#ifndef NET_DEVICENETPROTOCOL_H
#define NET_DEVICENETPROTOCOL_H

#include <cstdint>
#include "../../../ciptypes.hpp"
#include "./can_link/NET_CanInterface.hpp"

class NET_DeviceNetProtocol
{
public:
	typedef enum
	{
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
	}kDeviceNetErrorCodes;
private:
	typedef enum
	{
		NON_EXISTENT, SEND_DUPLICATE_MAC_REQ, WAIT_DUPLICATE_MAC_REQ, COMM_FAULT, ON_LINE
	}DeviceNetStates;
	int num_retries;
	bool quick_connect;
	void state_machine();

    bool transmit_dup_request();
    bool receive_msg(int timerVal);
    int timerVal;
    bool check_msg_4_dup();
    bool check_can_bus_off();
    bool discard_msg();
    bool comm_fault_request_msg_received();
    bool comm_fault_request();
    bool check_network_power();
    bool check_dup_resp_msg();
    bool check_dup_req_msg();
    bool transmit_dup_mac_response();
    bool transmit_mac_check_req_msg();
    int BOI;
    
    

	// define connection states
	//#define NON_EXISTENT						0x00
	#define CONFIGURING						0x01
	#define WAITING_FOR						0x02
	#define ESTABLISHED						0x03
	#define TIMED_OUT							0x04
	#define DEFERRED							0x05
	typedef enum
	{
		//
		//	Abreviations meaning
		//	Prefixes				Infixes						Sufixes
		//	slv_ = slave			_cyc_ = cyclic				_res = response
		//	mstr_ = master  		_st8chng_ = state change	_req = request
		//	g2only_ = only G2   	_mcast_ = multicast			_cmd = command
		//														_ack = acknowledge 


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
	}kDeviceNetMessageId;

	//static stuff
	static int dnet_identifier_group_check(uint32_t* can_id);
	static int dnet_identifier_group_match(struct can_frame* frame_rd);
	static int global_timer[10];

	CipUsint * id;

	//instances stuff
	#define BUFSIZE				80    			// size of message buffers
	DeviceNetStates state = NON_EXISTENT;
	char rcve_index, xmit_index;
	char my_rcve_fragment_count, my_xmit_fragment_count;
	int produced_conxn_size, consumed_conxn_size;
	char xmit_fragment_buf[BUFSIZE];
	char rcve_fragment_buf[BUFSIZE];
	char ack_timeout_counter;

public:
	void io_produce_message(char response[]);
	void explicit_produce_message(char response[]);
	int io_consume_message(char request[]);
	int explicit_consume_message(char request[]);
	void enframe_and_send(char message[]);
	NET_CanInterface * associated_interface;

	typedef enum {
		S0 = NET_CanInterface::S4,//125k
		S1 = NET_CanInterface::S5,//250k
		S2 = NET_CanInterface::S6 //500k
	}kDeviceNETBaudRate;

	NET_DeviceNetProtocol(CipUsint * id = nullptr, char state = NON_EXISTENT)
	{
		id = id;
		state = state;
	};
	~NET_DeviceNetProtocol() {};

};

#endif //NET_DEVICENETPROTOCOL_H