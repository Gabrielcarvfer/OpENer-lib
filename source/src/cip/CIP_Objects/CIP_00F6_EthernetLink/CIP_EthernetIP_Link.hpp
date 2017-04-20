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
class CIP_EthernetIP_Link;
class CIP_EthernetIP_Link : public CIP_Object<CIP_EthernetIP_Link>
{
public:
    /** @brief Initialize the Ethernet Link Objects data
    */
    static CipStatus Init();
    static CipStatus Shut();
    CIP_TCPIP_Interface * associatedInterface;
private:
    //Definitions
    typedef struct
    {
        CipUdint interface_speed;
        CipUdint interface_flags;
        CipUsint physical_address[6];
    } CipEthernetLinkObject;

    typedef union
    {
        CipUdint value;
        struct
        {
            CipUdint link_status:1;
            CipUdint duplex:1; //1 if full, 0 if half
            CipUdint negotiation_status:2;
            CipUdint manual_setting_requires_reset:1;
            CipUdint local_hardware_fault:1;
            CipUdint reserved:26;
        }bitfield_u;
    }interface_flags_t;

    typedef enum {
        interface_flag_linkStatus_inactive                   = 0,
        interface_flag_linkStatus_active                     = 1,
        interface_flag_duplex_half                           = 0,
        interface_flag_duplex_full                           = 1,
        interface_flag_negotiationStatus_autoInProgress      = 0,
        interface_flag_negotiationStatus_autoFailed          = 1,
        interface_flag_negotiationStatus_autoFailedSpeedDetected = 2,
        interface_flag_negotiationStatus_autoSucceded        = 3,
        interface_flag_negotiationStatus_autoNotAttempted    = 4,
        interface_flag_manualSettingRequiresReset_no         = 0, // autonegotiate
        interface_flag_manualSettingRequiresReset_yes        = 1, // requires that a reset service is issued
        interface_flag_localHardwareFault_notDetected        = 0,
        interface_flag_localHardwareFault_detected           = 1
    } interface_flags_e;

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

    typedef union
    {
        CipUint value;
        struct{
            CipUint auto_negotiate:1;
            CipUint forced_duplex_mode:1;
            CipUint reserved:14;
        }bitfield_u;
    }interface_control_bits_t;

    typedef struct {
        interface_control_bits_t Control_bits;
        CipUint Forced_interface_speed;
    }interface_control_t;

    typedef enum {
        kEthIPOriginalRev         = 1,
        kEthIPInterfaceControlRev = 2,
        kEthIPMultiplePortRev     = 3
    } ethip_link_revision_e;



    typedef enum {
        interface_control_autoNegotiationDisabled = 0,
        interface_control_autoNegotiationEnabled  = 1,

        //if autonegotiate = 0
        //  forceDuplexMode = 0 indicate that half duplex should be used
        //  forceDuplexMode = 1 indicate that full duplex should be used
        //if autonegotiate = 1
        // interfaces that dont support requested duplex results in GRC 0x09 (invalid attribute)
        // forceDuplexMode = 1 results in GRC 0x0C (object state conflict)
        interface_control_forceDuplexModeOff      = 0,
        interface_control_forceDuplexModeOn       = 1,
    } ethip_control_bits_e;

    //Methods
    void ConfigureMacAddress(const CipUsint* mac_address);


    //Instance attributes
    CipUdint Interface_speed;
    interface_flags_t Interface_flags;
    CipUsint Physical_address[6];
    interface_counters_t Interface_counters;
    media_counters_t Media_counters;
    interface_control_t Interface_control;
    CipUsint Interface_type;
    CipUsint Interface_state;
    CipUsint Admin_state;
    CipShortString Interface_label;


    CipEthernetLinkObject g_ethernet_link;

private:
    void * retrieveAttribute(CipUsint attributeNumber);
    CipStatus retrieveService(CipUsint serviceNumber, CipMessageRouterRequest_t *req, CipMessageRouterResponse_t *resp);
};

#endif /* OPENER_CIPETHERNETLINK_H_*/
