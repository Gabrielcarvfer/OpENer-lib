/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved. 
 *
 ******************************************************************************/
#include "CIP_MessageRouter.h"
#include "../connection_stack/CIP_Common.h"
#include "../../trace.h"


CipStatus CIP_MessageRouter::CipMessageRouterInit()
{
    classAttributesNum = 0;
    instanceAttributesNum = 0;

    classServicesNum = 0;
    instanceAttributesNum = 0;

    maxNumOfInstances = 1;

    revision = 1;
    class_name = "message router";
    /*message_router = CIP_ClassInstance(kCipMessageRouterClassCode, / class ID
                                                       0, /* # of class attributes
                                              0xffffffff, /* class getAttributeAll mask
                                                       0, /* # of class services
                                                       0, /* # of instance attributes
                                              0xffffffff, /* instance getAttributeAll mask
                                                       0, /* # of instance services
                                                       1, /* # of instances
                                        "message router", /* class name
                                                       1  /* revision
                              ); 
*/


    /* reserved for future use -> set to zero */
    g_message_router_response.reserved = 0;

    // set reply buffer, using a fixed buffer (about 100 bytes)
    g_message_router_response.data = g_message_data_reply_buffer; 

    return kCipStatusOk;
}
/*
CIP_ClassInstance* CIP_MessageRouter::GetRegisteredObject(CipUdint class_id)
{
    // for each entry in list
    for(auto ptr : message_router_registered_classes) 
    {
      // return registration node if it matches class ID
      if (*ptr->class_id == class_id)
        return ptr;
    } 

    return 0;  
}*/

CipStatus CIP_MessageRouter::RegisterCIPClass(CIP_ClassInstance* CIP_ClassInstance)
{
    /*
    ret = message_router_registered_classes.insert(message_router_registered_classes.count(), CIP_ClassInstance);

    if (ret::second)
        return kCipStatusOk;
    else
        return kCipStatusError;
        */
}

CipStatus CIP_MessageRouter::NotifyMR(CipUsint* data, int data_length)
{
    CipStatus cip_status = kCipStatusOkSend;
    CipByte nStatus;

    g_message_router_response.data = g_message_data_reply_buffer; /* set reply buffer, using a fixed buffer (about 100 bytes) */

    OPENER_TRACE_INFO("notifyMR: routing unconnected message\n");
    /* error from create MR structure*/
    nStatus = CreateMessageRouterRequestStructure(data, data_length, &g_message_router_request);

    if ( kCipErrorSuccess != nStatus )
    { 
        OPENER_TRACE_ERR("notifyMR: error from createMRRequeststructure\n");

        g_message_router_response.general_status = nStatus;
        g_message_router_response.size_of_additional_status = 0;
        g_message_router_response.reserved = 0;
        g_message_router_response.data_length = 0;
        g_message_router_response.reply_service = (0x80 | g_message_router_request.service);
    } 
    else 
    {
        /* forward request to appropriate Object if it is registered*/
        CIP_ClassInstance* registered_object;

        registered_object = GetRegisteredObject(g_message_router_request.request_path.class_id);
        if (registered_object == 0) 
        {
            OPENER_TRACE_ERR(
                "notifyMR: sending CIP_ERROR_OBJECT_DOES_NOT_EXIST reply, class id 0x%x is not registered\n",
                (unsigned)g_message_router_request.request_path.class_id);

            g_message_router_response.general_status = kCipErrorPathDestinationUnknown; /*according to the test tool this should be the correct error flag instead of CIP_ERROR_OBJECT_DOES_NOT_EXIST;*/
            g_message_router_response.size_of_additional_status = 0;
            g_message_router_response.reserved = 0;
            g_message_router_response.data_length = 0;
            g_message_router_response.reply_service = (CipUsint)(0x80 | g_message_router_request.service);
        } 
        else 
        {
            /* call notify function from Object with ClassID (gMRRequest.RequestPath.ClassID)
            object will or will not make an reply into gMRResponse*/
            g_message_router_response.reserved = 0;
            //OPENER_ASSERT(NULL != registered_object->CIP_ClassInstance);

            OPENER_TRACE_INFO("notifyMR: calling notify function of class '%s'\n",
                registered_object->CIP_ClassInstance->class_name);

            //todo:cip_status = NotifyClass(registered_object->CIP_ClassInstance, &g_message_router_request, &g_message_router_response);

#ifdef OPENER_TRACE_ENABLED
            switch(cip_status)
            {
                case (kCipStatusError):
                    OPENER_TRACE_ERR("notifyMR: notify function of class '%s' returned an error\n", registered_object->CIP_ClassInstance->class_name);
                    break;
                case (kCipStatusOK):
                    OPENER_TRACE_INFO("notifyMR: notify function of class '%s' returned no reply\n", registered_object->CIP_ClassInstance->class_name);
                    break;
                default:
                    OPENER_TRACE_INFO("notifyMR: notify function of class '%s' returned a reply\n", registered_object->CIP_ClassInstance->class_name);
            }
#endif
        }
    }
    return cip_status;
}

CipError CIP_MessageRouter::CreateMessageRouterRequestStructure(CipUsint* data, CipInt data_length, CipMessageRouterRequest* message_router_request)
{
    int number_of_decoded_bytes;

    message_router_request->service = *data;
    data++; /*TODO: Fix for 16 bit path lengths (+1 */
    data_length--;

    number_of_decoded_bytes = CIP_Common::DecodePaddedEPath( &(message_router_request->request_path), &data );

    if (number_of_decoded_bytes < 0) 
    {
        return kCipErrorPathSegmentError;
    }

    message_router_request->data = data;
    message_router_request->data_length = data_length - number_of_decoded_bytes;

    if (message_router_request->data_length < 0)
        return kCipErrorPathSizeInvalid;
    else
        return kCipErrorSuccess;
}

void CIP_MessageRouter::DeleteAllClasses(void)
{
    /*TODO: fix
    while (message_router_registered_classes.size() != 0) 
    {
        for (auto class_instances_set : CIP_ClassInstance::CIP_Object_set)
        {
            int class_id = *class_instances_set->class_id;

            //Removing all instances from each class
            for (auto instance : class_instances_set)
            {
                for (auto attribute : instance.attributes)
                {
                    delete attribute;
                }

                for (auto services : instance.services)
                {
                    delete services;
                }

                delete instance;
            }
            //Removing class
            for (auto attribute : CIP_ClassInstance::CIP_Class_set[class_id].attributes)
            {
                delete attribute;
            }

            for (auto services : CIP_ClassInstance::CIP_Class_set[class_id].attributes)
            {
                delete services;
            }
            
            //Remove main class and instances set
            CIP_ClassInstance::CIP_Class_set.erase(class_id);
            instances_classes_set.erase(class_id);


        }
    }
     */
}
