/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#pragma once

#include "connection_stack/CIP_Connection.h"
#include "ciptypes.h"

void InitializeIoConnectionData(void);

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
CIP_Connection* GetIoConnectionForConnectionData(
    CIP_Connection* connection_object, CipUint* extended_error);

/** @brief Check if there exists already an exclusive owner or listen only connection
 *         which produces the input assembly.
 *
 *  @param input_point the Input point to be produced
 *  @return if a connection could be found a pointer to this connection if not NULL
 */
CIP_Connection* GetExistingProducerMulticastConnection(CipUdint input_point);

/** @brief check if there exists an producing multicast exclusive owner or
 * listen only connection that should produce the same input but is not in charge
 * of the connection.
 *
 * @param input_point the produced input
 * @return if a connection could be found the pointer to this connection
 *      otherwise NULL.
 */
CIP_Connection* GetNextNonControlMasterConnection(CipUdint input_point);

/** @brief Close all connection producing the same input and have the same type
 * (i.e., listen only or input only).
 *
 * @param input_point  the input point
 * @param instance_type the connection application type
 */
void CloseAllConnectionsForInputWithSameType(CipUdint input_point, CIP_Connection::ConnectionType instance_type);

/**@ brief close all open connections.
 *
 * For I/O connections the sockets will be freed. The sockets for explicit
 * connections are handled by the encapsulation layer, and freed there.
 */
void CloseAllConnections(void);

/** @brief Check if there is an established connection that uses the same
 * config point.
 *
 * @param config_point The configuration point
 * @return true if connection was found, otherwise false
 */
CipBool ConnectionWithSameConfigPointExists(CipUdint config_point);

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
void ConfigureExclusiveOwnerConnectionPoint(unsigned int connection_number, unsigned int output_assembly_id, unsigned int input_assembly_id, unsigned int configuration_assembly_id);


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
void ConfigureInputOnlyConnectionPoint(unsigned int connection_number, unsigned int output_assembly_id, unsigned int input_assembly_id, unsigned int configuration_assembly_id);

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
void ConfigureListenOnlyConnectionPoint(unsigned int connection_number, unsigned int output_assembly_id, unsigned int input_assembly_id, unsigned int configuration_assembly_id);
