/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_CIPETHERNETLINK_H_
#define OPENER_CIPETHERNETLINK_H_

#include "../../ciptypes.hpp"
#include "../template/CIP_Object.hpp"
#include "cip/CIP_Objects/CIP_00F5_TCPIP_Interface/CIP_TCPIP_Interface.hpp"

class CIP_EthernetIP_Link : public CIP_Object<CIP_EthernetIP_Link>
{
public:
    /** @brief Initialize the Ethernet Link Objects data
    */
    static CipStatus Init();
    static CipStatus Shutdown();
    CIP_TCPIP_Interface * associatedInterface;
private:
    //Definitions
    typedef struct {
        CipUdint interface_speed;
        CipUdint interface_flags;
        CipUsint physical_address[6];
    } CipEthernetLinkObject;

    typedef struct {
        CipUdint In_octets;
        CipUdint In_ucast_packets;
        CipUdint In_nucast_packets;
        CipUdint In_discards;
        CipUdint In_errors;
        CipUdint In_unknown_protos;
        CipUdint Out_octets;
        CipUdint Out_ucast_packets;
        CipUdint Out_nucast_packets;
        CipUdint Out_discards;
        CipUdint Out_errors;
    }interface_counters_t;

    typedef struct {
        CipUdint Alignment_errors;
        CipUdint Fcs_errors;
        CipUdint Single_collisions;
        CipUdint Multiple_collisions;
        CipUdint Sqe_test_errors;
        CipUdint Deferred_transmissions;
        CipUdint Late_collisions;
        CipUdint Excessive_collisions;
        CipUdint Mac_transmit_errors;
        CipUdint Carrier_sense_errors;
        CipUdint Frame_too_long;
        CipUdint Mac_receive_errors;
    }media_counters_t;

    typedef struct {
        CipWord Control_bits;
        CipUint Forced_interface_speed;
    }interface_control_t;

    typedef enum {
        kEthIPOriginalRev         = 1,
        kEthIPInterfaceControlRev = 2,
        kEthIPMultiplePortRev     = 3
    } ethip_link_revision_e;

    typedef enum {
        kEthIpFlagsLinkStatus                               = 0,
        kEthIpFlagsHalfFullDuples                           = 1,
        kEthIpFlagsNegotiationStatusAutoInProgress          = SET_BIT_N_TO_X (2,0),
        kEthIpFlagsNegotiationStatusAutoFailed              = SET_BIT_N_TO_X (2,1),
        kEthIpFlagsNegotiationStatusAutoFailedSpeedDetected = SET_BIT_N_TO_X (3,0),
        kEthIpFlagsNegotiationStatusAutoSucess              = SET_BIT_N_TO_X (3,1),
        kEthIpFlagsNegotiationStatusAutoNotAttempted        = SET_BIT_N_TO_X (4,1),
        kEthIpFlagsManualSettingRequiresReset               = SET_BIT_N_TO_X (5,1),
        kEthIpFlagsLocalHardwareFault                       = SET_BIT_N_TO_X (6,1)
    } ethip_link_flags_e;

    typedef enum {
        kEthIpControlBitsAutoNegotiationDisabled = 0,
        kEthIpControlBitsAutoNegotiationEnabled  = 1,
        kEthIpControlBitsForcedDuplexModeHalf    = SET_BIT_N_TO_X (2,0),
        kEthIpControlBitsForcedDuplexModeFull    = SET_BIT_N_TO_X (2,1),
    } ethip_control_bits_e;

    //Methods
    void ConfigureMacAddress(const CipUsint* mac_address);


    //Instance attributes
    CipUdint Interface_speed;
    CipDword Interface_flags;
    CipUsint Physical_address[6];
    interface_counters_t Interface_counters;
    media_counters_t Media_counters;
    interface_control_t Interface_control;
    CipUsint Interface_type;
    CipUsint Interface_state;
    CipUsint Admin_state;
    CipShortString Interface_label;


    CipEthernetLinkObject g_ethernet_link;
    CipStatus InstanceServices(int service, CipMessageRouterRequest * msg_router_request,CipMessageRouterResponse* msg_router_response);

};

#endif /* OPENER_CIPETHERNETLINK_H_*/
