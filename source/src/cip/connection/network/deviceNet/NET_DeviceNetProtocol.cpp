#include <cstring>
#include <NET_DeviceNetProtocol.hpp>
#include <can_link/NET_CanInterface.hpp>
//
//	Get datagram group from CAN identifier
//
int NET_DeviceNetProtocol::dnet_identifier_group_check(uint32_t* can_id)
{

    //Check if identifier matches with Message Group 1
    if (*can_id <= 0x3ff) 
	{
        return 1;
    }

    //Check if identifier matches with Message Group 2
    if (*can_id <= 0x5ff) 
	{
        return 2;
    }

    //Check if identifier matches with Message Group 3
    if (*can_id <= 0x7bf)
	{
        return 3;
    }

    //Check if identifier matches with Message Group 4
    if (*can_id <= 0x7cf) 
	{
        return 4;
    }

    //If failed every test, invalid identifier
    return -1;
}

//
//	Get info from CAN identifier based on Group ID
//
int NET_DeviceNetProtocol::dnet_identifier_group_match(struct can_frame* frame_rd)
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

void NET_DeviceNetProtocol::state_machine()
{
	switch (state)
	{
        //Comming from device power lost
		case (NON_EXISTENT):
			//device power up
			num_retries = 0;
			state = SEND_DUPLICATE_MAC_REQ;
			break;
		case (SEND_DUPLICATE_MAC_REQ):
			if (transmit_dup_request())
			{
				if (quick_connect)
				{
					state = ON_LINE;
				}
				else
				{
					state = WAIT_DUPLICATE_MAC_REQ;
				}
			}
			else
			{
				//If dupMacCheck req/resp received
				//CAN bus-Off detected
				//Network power offline
				state = COMM_FAULT;
			}
			break;
		case (WAIT_DUPLICATE_MAC_REQ):
			bool timeout;
            timeout = receive_msg(timerVal);
			if (timeout)
			{
				num_retries++;
			}
			//If duplicate MAC is not received and can bus is offline
			if (check_msg_4_dup() | check_can_bus_off())
			{
				state = COMM_FAULT;
			}
			//If duplicate MAC is not received and can is online
			else
			{
				//In case retries num is equal one, then continue
				if (num_retries == 1)
				{
					state = ON_LINE;
				}
				//Else, you keep checking and discarding messages
				else
				{
                    discard_msg();
				}
			}
			break;
		case (COMM_FAULT):
            comm_fault_request_msg_received();
			//Manual intervention
			//Comm_fault_request_message
			//	change_mac_msg
			//	change_mac_to_net_mac
			//Network power restored
			if (comm_fault_request() | check_network_power())
			{
				num_retries = 0;
				state = SEND_DUPLICATE_MAC_REQ;
			}
			break;
		case (ON_LINE):
			if (check_can_bus_off() & BOI == 1)
			{
				num_retries = 0;
				state = SEND_DUPLICATE_MAC_REQ;
			}
			if (check_dup_resp_msg() | (check_can_bus_off() & BOI == 0))
			{
				state = COMM_FAULT;
			}
			if (check_dup_req_msg())
                transmit_dup_mac_response();

			if (quick_connect & timeout & num_retries == 0)
			{
				num_retries++;
                transmit_mac_check_req_msg();
			}

			break;
		default:
			//Undefined state

			break;
	}
}


#define NO_RESPONSE			0
#define LENGTH 				BUFSIZE - 1		// this location holds length of message
#define MESSAGE_TAG			BUFSIZE - 2    // this location holds a message tag

//Define message fragment values
#define  FIRST_FRAG						0x00
#define  MIDDLE_FRAG						0x40
#define  LAST_FRAG						0x80
#define  ACK_FRAG							0xC0

// define message tags which can be used to identify message
#define  RECEIVED_ACK					0x01
#define  SEND_ACK							0x02
#define  ACK_TIMEOUT						0x03
#define	ACK_ERROR						0x04
#define OK						1



// Define Instance IDs and TIMER numbers
#define  EXPLICIT							0x01
#define  IO_POLL							0x02
#define  BIT_STROBE						0x03
#define  COS_CYCLIC						0x04
#define  ACK_WAIT							0x05
#define  UPDATE							0x06


int NET_DeviceNetProtocol::explicit_consume_message(char request[])
{
	char i, length, fragment_count, fragment_flg;
    int fragment_type;
	char temp_buffer[BUFSIZE];

	/*
	if (state = DEFERRED)
		state = ESTABLISHED;
	if (state != ESTABLISHED)
		return NO_RESPONSE;
		*/

	// From this point on we are dealing with an Explicit message
	// If it is not fragmented, reset frag counters to initial state
	// and process the current incoming message.
	if ((request[0] & 0x80) == 0)
	{
		rcve_index = 0;
		xmit_index = 0;
		my_rcve_fragment_count = 0;
		my_xmit_fragment_count = 0;
		ack_timeout_counter = 0;
		global_timer[ACK_WAIT] = 0;
		return OK;
	}

	// At this point we are dealing with either an explicit fragment
	// or an ACK to an explicit message we sent earlier
	fragment_type = request[1] & 0xC0;
	fragment_count = request[1] & 0x3F;
	length = request[LENGTH];


	switch (fragment_type)
	{
	case (ACK_FRAG):
		// Received an ack from the master to an explicit fragment I sent earlier
		// Send the request to the link producer along with a tag so it knows
		// what do do with the message
		request[MESSAGE_TAG] = RECEIVED_ACK;
		explicit_produce_message(request);
		break;


	case FIRST_FRAG:
		if (fragment_count != 0)
		{
			// Reset fragment counters to initial state and drop fragment
			rcve_index = 0;
			my_rcve_fragment_count = 0;
			return NO_RESPONSE;
		}

		// get rid of any existing fragments and process first fragment
		rcve_index = 0;
		my_rcve_fragment_count = 0;
		memset(rcve_fragment_buf, 0, BUFSIZE);
		rcve_fragment_buf[rcve_index] = request[0] & 0x7F; // remove fragment flag
		rcve_index++;
		for (i = 2; i < length; i++)      // do not copy fragment info
		{
			rcve_fragment_buf[rcve_index] = request[i];
			rcve_index++;
		}

		// Check to see if Master has exceeded byte count limit
		if (rcve_index > consumed_conxn_size)
		{
			rcve_index = 0;                       // reset fragment counters
			my_rcve_fragment_count = 0;
			request[MESSAGE_TAG] = ACK_ERROR;
			explicit_produce_message(request);
		}
		else  // fragment checked out OK so send ACK
		{
			request[MESSAGE_TAG] = SEND_ACK;
			explicit_produce_message(request);
			my_rcve_fragment_count++;
		}
		break;



	case (MIDDLE_FRAG):
		if (my_rcve_fragment_count == 0) 
			return NO_RESPONSE;  // just drop fragment

															  // See if Master has sent the same fragment again.  If so just send ACK
		if (fragment_count == (my_rcve_fragment_count - 1))
		{
			request[MESSAGE_TAG] = SEND_ACK;
			explicit_produce_message(request);
		}
		// See if received fragment count does not agree with my count
		// If so, reset to beginning
		else if (fragment_count != my_rcve_fragment_count)
		{
			rcve_index = 0;
			my_rcve_fragment_count = 0;
		}

		else  // Fragment was validated so process it
		{
			// Copy the fragment to reassembly buffer, omit first 2 bytes
			for (i = 2; i < length; i++)
			{
				rcve_fragment_buf[rcve_index] = request[i];
				rcve_index++;
			}
			// Check to see if Master has exceeded byte count limit
			if (rcve_index > consumed_conxn_size)
			{
				rcve_index = 0;
				my_rcve_fragment_count = 0;
				request[MESSAGE_TAG] = ACK_ERROR;
				explicit_produce_message(request);
			}
			else  // send good ACK back to Master
			{
				request[MESSAGE_TAG] = SEND_ACK;
				explicit_produce_message(request);
				my_rcve_fragment_count++;
			}
		}
		break;



	case (LAST_FRAG):
		if (my_rcve_fragment_count == 0)
		{
			return NO_RESPONSE;  // just drop fragment

		}
		// See if received fragment count does not agree with my count
		// If so, reset to beginning
		else if (fragment_count != my_rcve_fragment_count)
		{
			rcve_index = 0;
			my_rcve_fragment_count = 0;
		}
		else  // Fragment was validated, so process it
		{
			// Copy the fragment to reassembly buffer, omit first 2 bytes
			for (i = 2; i < length; i++)
			{
				rcve_fragment_buf[rcve_index] = request[i];
				rcve_index++;
			}

			// Check to see if Master has exceeded byte count limit
			if (rcve_index > consumed_conxn_size)
			{
				rcve_index = 0;
				my_rcve_fragment_count = 0;
				memset(rcve_fragment_buf, 0, BUFSIZE);
				request[MESSAGE_TAG] = ACK_ERROR;
				explicit_produce_message(request);
			}
			else  // Fragment was OK, send ACK and reset everything
			{
				request[MESSAGE_TAG] = SEND_ACK;
				explicit_produce_message(request);
				// We are done receiving a fragmented explicit message
				// Copy from temporary buffer back to request buffer
				// and process complete Explicit message
				memcpy(request, rcve_fragment_buf, BUFSIZE);
				request[LENGTH] = rcve_index;
				rcve_index = 0;
				my_rcve_fragment_count = 0;
				memset(rcve_fragment_buf, 0, BUFSIZE);
				return OK;
			}
		}
		break;


	default:
		break;
	}

	return NO_RESPONSE;
}

int NET_DeviceNetProtocol::io_consume_message(char request[])
{
	// The Master should not send me any data in I/O message
	if (request[LENGTH] == 0) 
		return OK;
	else 
		return NO_RESPONSE;
}

void NET_DeviceNetProtocol::enframe_and_send(char message[])
{

	struct can_frame frame;
	char length = message[LENGTH];
	memcpy(&(frame.data),message, length);
	frame.can_dlc = length;
	frame.can_id = *id;

	associated_interface->send_frame(&frame);
}
void NET_DeviceNetProtocol::io_produce_message(char response[])
{
	enframe_and_send(response);
}


void NET_DeviceNetProtocol::explicit_produce_message(char response[])
{
	char length, bytes_left, i, fragment_count, ack_status;
	char copy[BUFSIZE];

	length = response[LENGTH];

	if (response[MESSAGE_TAG] == ACK_TIMEOUT)
	{
		ack_timeout_counter++;
		if (ack_timeout_counter == 1)
		{
			enframe_and_send(response);
			global_timer[ACK_WAIT] = 20;

		}
		if (ack_timeout_counter == 2)
		{
			// abort trying to send fragmented message
			xmit_index = 0;
			my_xmit_fragment_count = 0;
			ack_timeout_counter = 0;
			global_timer[ACK_WAIT] = 0;
		}
	}


	else if (response[MESSAGE_TAG] == RECEIVED_ACK)
	{
		fragment_count = response[1] & 0x3F;
		ack_status = response[2];
		if (my_xmit_fragment_count == fragment_count)
		{
			// If Master returned a bad ACK status, reset everything
			// and abort the attempt to send message
			if (ack_status != 0)
			{
				xmit_index = 0;
				my_xmit_fragment_count = 0;
				ack_timeout_counter = 0;
				global_timer[ACK_WAIT] = 0;
			}

			// Master's ACK was OK, so send next fragment unless we were done sending
			// Keep a copy of what we are sending
			else
			{
				if (xmit_index >= xmit_fragment_buf[LENGTH])
				{
					// got ACK to out final fragment so reset everything
					xmit_index = 0;
					my_xmit_fragment_count = 0;
					ack_timeout_counter = 0;
					global_timer[ACK_WAIT] = 0;
				}
				else
				{
					// Send another fragment
					// Figure out how many bytes are left to send
					bytes_left = xmit_fragment_buf[LENGTH] - xmit_index;
					my_xmit_fragment_count++;
					ack_timeout_counter = 0;
					global_timer[ACK_WAIT] = 20;				// restart timer for 1 second
																// Load the first byte of the fragment
					copy[0] = response[0] | 0x80;
					//pokeb(CAN_BASE, 0x67, copy[0]);
					if (bytes_left > 6)			// this is a middle fragment
					{
						copy[1] = MIDDLE_FRAG | my_xmit_fragment_count;
						//pokeb(CAN_BASE, 0x68, copy[1]);
						length = 8;
					}
					else   							// this is the last fragment
					{
						copy[1] = LAST_FRAG | my_xmit_fragment_count;
						//pokeb(CAN_BASE, 0x68, copy[1]);
						length = bytes_left + 2;
					}
					// Put in actual data
					for (i = 2; i < length; i++)  // put up to 6 more bytes in CAN chip
					{
						copy[i] = xmit_fragment_buf[xmit_index];
						//pokeb(CAN_BASE, (0x67 + i), copy[i]);
						xmit_index++;
					}
					copy[LENGTH] = length;
					/*
					pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
					pokeb(CAN_BASE, 0x61, 0x66);      // set msg object transmit request
					*/
				}
			}
		}
	}


	// Send this message in response to receiving an explicit fragment
	// from the Master that the link comsumer has validated as OK
	else if (response[MESSAGE_TAG] == SEND_ACK)
	{
		length = 3;												// This is a 3 byte message
		/*
		pokeb(CAN_BASE, 0x67, (response[0] | 0x80));
		pokeb(CAN_BASE, 0x68, (response[1] | ACK_FRAG));
		pokeb(CAN_BASE, 0x69, 0);								// ack status = OK
		pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
		pokeb(CAN_BASE, 0x61, 0x66);      					// set transmit request
		*/
	}



	// Send this message in response to receiving an explicit fragment
	// from the Master that exceeded the byte count limit for the connection
	else if (response[MESSAGE_TAG] == ACK_ERROR)
	{
		length = 3;												// This is a 3 byte message

		/*
		pokeb(CAN_BASE, 0x67, (response[0] | 0x80));
		pokeb(CAN_BASE, 0x68, (response[1] | ACK_FRAG));
		pokeb(CAN_BASE, 0x69, 1);								// ack status = TOO MUCH DATA
		pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config register
		pokeb(CAN_BASE, 0x61, 0x66);      					// set transmit request
		*/
	}



	else if (length <= 8)		// Send complete Explicit message
	{
		enframe_and_send(response);
	}



	else if (length > 8)       // Send first Explicit message fragment
	{
		// load explicit response into a buffer to keep around a while
		memcpy(xmit_fragment_buf, response, BUFSIZE);
		length = 8;
		xmit_index = 0;
		my_xmit_fragment_count = 0;
		ack_timeout_counter = 0;
		// Load first fragment into can chip object #3
		copy[0] = response[0] | 0x80;
		//pokeb(CAN_BASE, 0x67, copy[0]);
		xmit_index++;
		// Put in fragment info
		copy[1] = FIRST_FRAG | my_xmit_fragment_count;
		//pokeb(CAN_BASE, 0x68, copy[1]);
		// Put in actual data
		for (i = 2; i < 8; i++)  // put 6 more bytes in CAN chip
		{
			copy[i] = xmit_fragment_buf[xmit_index];
			//pokeb(CAN_BASE, (0x67 + i), copy[i]);
			xmit_index++;
		}
		copy[LENGTH] = length;
		//pokeb(CAN_BASE, 0x66, ((length << 4) | 0x08));	// load config resister
		//pokeb(CAN_BASE, 0x61, 0x66);      					// set msg object transmit request
		global_timer[ACK_WAIT] = 20;							// start timer to wait for ack
	}
}



//Messages format

//Acknowledge fragment (Explicit message only, after receiving each fragment
/*
 *             Contents
 * Byte   |7     |6      |5     |4    |3    |2    |1     |0     |
 * 0      |frag  |xid    |mac_id
 * 1      |fragment_type |fragment_count
 * 2      |ack_status
 *
 * Ack Status (0 = OK, 1 = exceeded receiver data limit, 2-FF reserved)
 * Fragmented (1 = true, 0 = false)
 * XID a.k.a. transaction ID ()
 * Fragment type (3 = explicit fragment)
 *
 */
char * ack_explicit_fragment(int mac_id, int xid, int fragment_count, int ack_status)
{
    char * msg = new char[3]();

    int fragment_type = 3;
    int fragmented = 1;

    //First byte
    msg[0] |= 0x03F & mac_id;
    msg[0] |= 0x040 & xid << 6;
    msg[0] |= 0x080 & fragmented << 7;

    //Second byte

    msg[1] |= 0x03F & fragment_count;
    msg[1] |= 0x0B0 & fragment_type;

    //Third byte
    msg[2] = ack_status;

    return msg;
}

//Open Explicit connection request
/*
 *             Contents
 * Byte   |7     |6      |5     |4    |3    |2    |1     |0     |
 * 0      |frag  |xid    |mac_id
 * 1      |RR    |service_code
 * 2      |request_message_body_format
 * 3      |group_select         |source_msg_id
 *
 * Fragmented (1 = true, 0 = false)
 * XID a.k.a. transaction ID ()
 * RR a.k.a. (0 = request, 1 = response)
 * Service code (4B = open explicit messaging connection service)
 * Request message body format (
 *                              DevNet XbitClassId/YbitInstanceID
 *                              0 = DevNet 8/8
 *                              1 = DevNet 8/16
 *                              2 = DevNet 16/16
 *                              3 = DevNet = 16/8
 *                              4 = CIP Path (Packed EPATH, variable size)
 *                              5-F = reserved
 *                              )
 * Group select (
 *               0 = msg_group1, 1 = msg_group2,
 *               2 = reserved, 3 = msg_group3,
 *               4-E = reserved, F = reserved for ping
 *               )
 *
 * Source Message ID (depends on group. 1 = ignored/set zero, 0 or 3 = msg_id client allocated from group 1 or 3 msg_id pool)
 */
char * open_explicit_conn_req(int mac_id, int xid, int req_msg_body_format, int source_msg_id, int group_select)
{
    char * msg = new char[4]();

    int fragmented = 0;
    int service_code = 0x04B;
    int RR = 0;

    //First byte
    msg[0] |= 0x03F & mac_id;
    msg[0] |= 0x040 & (xid << 6);
    msg[0] |= 0x080 & (fragmented << 7);

    //Second byte
    msg[1] |= 0x07F & service_code;
    msg[1] |= 0x080 & (RR << 7);

    //Third byte
    msg[2] |= 0x00F & req_msg_body_format;

    //Fourth byte
    msg[3] |= 0x00F & source_msg_id;
    msg[3] |= 0x0F0 & (group_select << 4);

    return msg;
}

//Open Explicit connection success response
/*
 *             Contents
 * Byte   |7     |6      |5     |4    |3    |2    |1     |0     |
 * 0      |frag  |xid    |mac_id
 * 1      |RR    |service_code
 * 2      |request_message_body_format
 * 3      |destination_msg_id   |source_msg_id
 * 4      |connection_instance_id (first half)
 * 5      |connection_instance_id (second half)
 *
 * Fragmented (1 = true, 0 = false)
 * XID a.k.a. transaction ID (meaningless if sent message dont expect response, server just echoes client)
 * RR a.k.a. request or response (0 = request, 1 = response)
 * Service code (4B = open explicit conn request)
 * Request message body format (
 *                              DevNet XbitClassId/YbitInstanceID
 *                              0 = DevNet 8/8
 *                              1 = DevNet 8/16
 *                              2 = DevNet 16/16
 *                              3 = DevNet = 16/8
 *                              4 = CIP Path (Packed EPATH, variable size)
 *                              5-F = reserved
 *                              )
 * Destination Message ID (depends on request group, 1 = used by client, this value was allocated in group 2, 0 or 3 ignore and set zero)
 * Source Message ID (depends on group. 1 = ignored/set zero, 0 or 3 = msg_id client allocated from group 1 or 3 msg_id pool)
 * Connection Instance ID (8LSB + 8MSB)
 */

char * open_explicit_conn_resp(int mac_id, int xid, int actual_msg_body_format, int source_msg_id, int dest_msg_id, int conn_instance_id)
{
    char * msg = new char[6]();

    int fragmented = 0;
    int service_code = 0x04B;
    int RR = 0;

    //First byte
    msg[0] |= 0x03F & mac_id;
    msg[0] |= 0x040 & (xid << 6);
    msg[0] |= 0x080 & (fragmented << 7);

    //Second byte
    msg[1] |= 0x07F & service_code;
    msg[1] |= 0x080 & (RR << 7);

    //Third byte
    msg[2] |= 0x00F & actual_msg_body_format;

    //Fourth byte
    msg[3] |= 0x00F & source_msg_id;
    msg[3] |= 0x0F0 & (dest_msg_id << 4);

    //Fifth byte
    msg[4] = (char) (0x0FF & conn_instance_id);

    //Sixth byte
    msg[5] = (char) ((0x0FF00 & conn_instance_id) >> 8);


    return msg;
}

//Close connection request
/*
 *             Contents
 * Byte   |7     |6      |5     |4    |3    |2    |1     |0     |
 * 0      |frag  |xid    |mac_id
 * 1      |RR    |service_code
 * 2      |connection_instance_id(first half)
 * 3      |connection_instance_id(second half)
 *
 * Fragmented (1 = true, 0 = false)
 * XID a.k.a. transaction ID ()
 * RR a.k.a. (0 = request, 1 = response)
 * Service code (4B = open explicit conn request)
 * Connection Instance ID (8LSB + 8MSB)
 *
 */
char * close_conn_req(int mac_id, int xid, int conn_instance_id)
{
    char * msg = new char[4]();

    int fragmented = 0;
    int service_code = 0x04C;
    int RR = 0;

    //First byte
    msg[0] |= 0x03F & mac_id;
    msg[0] |= 0x040 & (xid << 6);
    msg[0] |= 0x080 & (fragmented << 7);

    //Second byte
    msg[1] |= 0x07F & service_code;
    msg[1] |= 0x080 & (RR << 7);

    //Third byte
    msg[2] = (char) (0x0FF & conn_instance_id);

    //Fourth byte
    msg[3] = (char) ((0x0FF00 & conn_instance_id) >> 8);


    return msg;
}


//Close connection request
/*
 *             Contents
 * Byte   |7     |6      |5     |4    |3    |2    |1     |0     |
 * 0      |frag  |xid    |mac_id
 * 1      |RR    |service_code
 * 2      |connection_instance_id(first half)
 * 3      |connection_instance_id(second half)
 *
 * Fragmented (1 = true, 0 = false)
 * XID a.k.a. transaction ID ()
 * RR a.k.a. (0 = request, 1 = response)
 * Service code (4B = open explicit conn request)
 * Connection Instance ID (8LSB + 8MSB)
 *
 */
char * close_conn_resp(int mac_id, int xid)
{
    char * msg = new char[4]();

    int fragmented = 0;
    int service_code = 0x04C;
    int RR = 1;

    //First byte
    msg[0] |= 0x03F & mac_id;
    msg[0] |= 0x040 & (xid << 6);
    msg[0] |= 0x080 & (fragmented << 7);

    //Second byte
    msg[1] |= 0x07F & service_code;
    msg[1] |= 0x080 & (RR << 7);

    return msg;
}

//Actions of state machine
bool NET_DeviceNetProtocol::transmit_dup_request()
{
    return false;
}

bool NET_DeviceNetProtocol::receive_msg(int timerVal)
{
    return false;
}

bool NET_DeviceNetProtocol::check_msg_4_dup()
{
    return false;
}

bool NET_DeviceNetProtocol::check_can_bus_off()
{
    return false;
}

bool NET_DeviceNetProtocol::discard_msg()
{
    return false;
}

bool NET_DeviceNetProtocol::comm_fault_request_msg_received()
{
    return false;
}

bool NET_DeviceNetProtocol::comm_fault_request()
{
    return false;
}

bool NET_DeviceNetProtocol::check_network_power()
{
    return false;
}

bool NET_DeviceNetProtocol::check_dup_resp_msg()
{
    return false;
}

bool NET_DeviceNetProtocol::check_dup_req_msg()
{
    return false;
}

bool NET_DeviceNetProtocol::transmit_dup_mac_response()
{
    return false;
}

bool NET_DeviceNetProtocol::transmit_mac_check_req_msg()
{
    return false;
}