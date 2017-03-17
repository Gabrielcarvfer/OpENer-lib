/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIP_COMMON_H_
#define OPENER_CIP_COMMON_H_

/** @file cipcommon.h
 * Common CIP object interface
 */
#include "ciptypes.hpp"
#include <array>
#include <map>

class CIP_Common
{

	public:
		/** A buffer for holding the replay generated by explicit message requests
		 *  or producing I/O connections. These will use this buffer in the following
		 *  ways:
		 *    1. Explicit messages will use this buffer to store the data generated by the request
		 *    2. I/O Connections will use this buffer for the produced data
		 */
		static std::map <CipUsint,CipByteArray*> message_data_reply_buffer;

    virtual /** @brief Check if requested service present in class/instance and call appropriate service.
		 *
		 * @param class class receiving the message
		 * @param message_router_request request message
		 * @param message_router_response reply message
		 * @return
		 *     - EIP_OK_SEND    ... success
		 *     - EIP_OK  ... no reply to send back
		 *     - EIP_ERROR ... error
		 */
		CipStatus NotifyClass(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response)= 0;


		/** @brief Decodes padded EPath
		 *  @param epath EPath to the receiving element
		 *  @param message CIP Message to decode
		 *  @return Number of decoded bytes
		 */
		static int DecodePaddedEPath(CipEpath* epath, CipUsint* data);

		static void CipStackInit(CipUint unique_connection_id);
		static void ShutdownCipStack(void);
	/** @brief Produce the data according to CIP encoding onto the message buffer.
     *
     * This function may be used in own services for sending data back to the
     * requester (e.g., getAttributeSingle for special structs).
     *  @param cip_data_type the cip type to encode
     *  @param cip_data pointer to data value.
     *  @param cip_message pointer to memory where response should be written
     *  @return length of attribute in bytes
     *          -1 .. error
     */
	static int EncodeData(CipUsint cip_type, void* data, CipUsint* message);
	private:
		static const CipUint kCipUintZero = 0;
		static int EncodeEPath(CipEpath* epath, CipUsint* message);



		/** @brief Retrieve the given data according to CIP encoding from the message
		 * buffer.
		 *
		 * This function may be used in in own services for handling data from the
		 * requester (e.g., setAttributeSingle).
		 *  @param cip_data_type the CIP type to decode
		 *  @param cip_data pointer to data value to written.
		 *  @param cip_message pointer to memory where the data should be taken from
		 *  @return length of taken bytes
		 *          -1 .. error
		 */
		static int DecodeData(CipUsint cip_type, void* data, CipUsint* message);

};

#endif
