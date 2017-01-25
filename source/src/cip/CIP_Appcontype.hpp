/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#ifndef OPENER_APPCONTYPE_H_
#define OPENER_APPCONTYPE_H_

#include "CIP_Objects/template/CIP_Object.hpp"
#include "CIP_Objects/CIP_0006_ConnectionManager/CIP_ConnectionManager.hpp"
#include "ciptypes.hpp"
#include "../opener_user_conf.hpp"

class CIP_Appcontype;
class CIP_Appcontype : public CIP_Object<CIP_Appcontype>
{
public:
    static void InitializeIoConnectionData (void);

/** @brief check if for the given connection data received in a forward_open request
 *  a suitable connection is available.
 *
 *  If a suitable connection is found the connection data is transfered the
 *  application connection type is set (i.e., EConnType).
 *  @param connection_object connection data to be used
 *  @param extended_error if an error occurred this value has the according
 *     error code for the response
 *  @return
 *        - on success: A pointer to the connection object already containing the connection
 *          data given in pa_pstConnData.
 *        - on error: NULL
 */
    static const CIP_ConnectionManager *GetIoConnectionForConnectionData (const CIP_ConnectionManager *connection_object, CipUint *extended_error);

/** @brief Check if there exists already an exclusive owner or listen only connection
 *         which produces the input assembly.
 *
 *  @param input_point the Input point to be produced
 *  @return if a connection could be found a pointer to this connection if not NULL
 */
    static const CIP_ConnectionManager *GetExistingProducerMulticastConnection (CipUdint input_point);

/** @brief check if there exists an producing multicast exclusive owner or
 * listen only connection that should produce the same input but is not in charge
 * of the connection.
 *
 * @param input_point the produced input
 * @return if a connection could be found the pointer to this connection
 *      otherwise NULL.
 */
    static const CIP_ConnectionManager *GetNextNonControlMasterConnection (CipUdint input_point);

/** @brief Close all connection producing the same input and have the same type
 * (i.e., listen only or input only).
 *
 * @param input_point  the input point
 * @param instance_type the connection application type
 */
    static void CloseAllConnectionsForInputWithSameType (CipUdint input_point, CIP_ConnectionManager::ConnectionType instance_type);

/**@ brief close all open connections.
 *
 * For I/O connections the sockets will be freed. The sockets for explicit
 * connections are handled by the encapsulation layer, and freed there.
 */
    static void CloseAllConnections (void);

/** @brief Check if there is an established connection that uses the same
 * config point.
 *
 * @param config_point The configuration point
 * @return true if connection was found, otherwise false
 */
    static CipBool ConnectionWithSameConfigPointExists (CipUdint config_point);

/* @brief Configures the connection point for an exclusive owner connection.
*
* @param connection_number The number of the exclusive owner connection. The
        *        enumeration starts with 0. Has to be smaller than
        *        OPENER_CIP_NUM_EXLUSIVE_OWNER_CONNS.
* @param output_assembly_id ID of the O-to-T point to be used for this
* connection
* @param input_assembly_id ID of the T-to-O point to be used for this
* connection
* @param configuration_assembly_id ID of the configuration point to be used for
* this connection
*/
    static void ConfigureExclusiveOwnerConnectionPoint (unsigned int connection_number, unsigned int output_assembly_id,
                                                 unsigned int input_assembly_id,
                                                 unsigned int configuration_assembly_id);


/* @brief Configures the connection point for an input only connection.
 *
 * @param connection_number The number of the input only connection. The
 *        enumeration starts with 0. Has to be smaller than
 *        OPENER_CIP_NUM_INPUT_ONLY_CONNS.
 * @param output_assembly_id ID of the O-to-T point to be used for this
 * connection
 * @param input_assembly_id ID of the T-to-O point to be used for this
 * connection
 * @param configuration_assembly_id ID of the configuration point to be used for
 *this connection
 */
    static void ConfigureInputOnlyConnectionPoint (unsigned int connection_number, unsigned int output_assembly_id,
                                            unsigned int input_assembly_id, unsigned int configuration_assembly_id);

/* @brief Configures the connection point for a listen only connection.
 *
 * @param connection_number The number of the input only connection. The
 *        enumeration starts with 0. Has to be smaller than
 *        OPENER_CIP_NUM_LISTEN_ONLY_CONNS.
 * @param output_assembly_id ID of the O-to-T point to be used for this
 * connection
 * @param input_assembly_id ID of the T-to-O point to be used for this
 * connection
 * @param configuration_assembly_id ID of the configuration point to be used for
 * this connection
 */
    static void ConfigureListenOnlyConnectionPoint (unsigned int connection_number, unsigned int output_assembly_id,
                                             unsigned int input_assembly_id, unsigned int configuration_assembly_id);


    typedef struct {
        unsigned int output_assembly; /**< the O-to-T point for the connection */
        unsigned int input_assembly; /**< the T-to-O point for the connection */
        unsigned int config_assembly; /**< the config point for the connection */
        CIP_ConnectionManager *connection_data; /**< the connection data, only one connection is allowed per O-to-T point*/
    } ExclusiveOwnerConnection;

    typedef struct {
        unsigned int output_assembly; /**< the O-to-T point for the connection */
        unsigned int input_assembly; /**< the T-to-O point for the connection */
        unsigned int config_assembly; /**< the config point for the connection */
        CIP_ConnectionManager *connection_data;//[OPENER_CIP_NUM_INPUT_ONLY_CONNS_PER_CON_PATH]; /*< the connection data */
    } InputOnlyConnection;

    typedef struct {
        unsigned int output_assembly; /**< the O-to-T point for the connection */
        unsigned int input_assembly; /**< the T-to-O point for the connection */
        unsigned int config_assembly; /**< the config point for the connection */
        CIP_ConnectionManager *connection_data;//[OPENER_CIP_NUM_LISTEN_ONLY_CONNS_PER_CON_PATH]; /**< the connection data */
    } ListenOnlyConnection;



private:

    static ExclusiveOwnerConnection *g_exlusive_owner_connections;

    static InputOnlyConnection *g_input_only_connections;

    static ListenOnlyConnection *g_listen_only_connections;

    static const CIP_ConnectionManager* GetExclusiveOwnerConnection(const CIP_ConnectionManager* connection_object, CipUint* extended_error);

    static const CIP_ConnectionManager* GetInputOnlyConnection(const CIP_ConnectionManager* connection_object, CipUint* extended_error);

    static const CIP_ConnectionManager* GetListenOnlyConnection(const CIP_ConnectionManager* connection_object, CipUint* extended_error);
};
#endif
