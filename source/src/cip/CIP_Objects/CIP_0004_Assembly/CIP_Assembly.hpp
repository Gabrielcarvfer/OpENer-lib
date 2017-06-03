/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef CIP_CLASSES_ASSEMBLY_H
#define CIP_CLASSES_ASSEMBLY_H

#include "../../ciptypes.hpp"
#include "../template/CIP_Object.hpp"
class CIP_Assembly;
class CIP_Assembly : public CIP_Object<CIP_Assembly>
{
public:
        static CipStatus Create(CipByte* data = nullptr, CipUint data_length = 0);

		/** @brief Setup the Assembly object
		 *
		 * Creates the Assembly Class with zero instances and sets up all services.
		 */

		static CipStatus Init (void);


		/** @brief clean up the data allocated in the assembly object instances
		 *
		 * Assembly object instances allocate per instance data to store attribute 3.
		 * This will be freed here. The assembly object data given by the application
		 * is not freed neither the assembly object instances. These are handled in the
		 * main shutdown function.
		 */
		static CipStatus Shut (void);

		/** @brief notify an Assembly object that data has been received for it.
		 *
		 *  The data will be copied into the assembly objects attribute 3 and
		 *  the application will be informed with the IApp_after_assembly_data_received function.
		 *
		 *  @param instance the assembly object instance for which the data was received
		 *  @param data pointer to the data received
		 *  @param data_length number of bytes received
		 *  @return
		 *     - EIP_OK the received data was okay
		 *     - EIP_ERROR the received data was wrong
		 */
		CipStatus NotifyAssemblyConnectedDataReceived(CipUsint* data, CipUint data_length);

	private:
		CipByteArray assemblyByteArray;
        /** @brief Implementation of the SetAttributeSingle CIP service for Assembly
             *          Objects.
             *  Currently only supports Attribute 3 (CIP_BYTE_ARRAY) of an Assembly
             */
        //CipStatus SetAssemblyAttributeSingle(CipMessageRouterRequest_t* message_router_request, CipMessageRouterResponse_t* message_router_response);

	    void * retrieveAttribute(CipUsint attributeNumber);
		CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);
};

#endif
