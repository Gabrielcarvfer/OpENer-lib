/*******************************************************************************
 * Copyright (c) 2011, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#ifndef OPENER_CIPCLASS3CONNECTION_H_
#define OPENER_CIPCLASS3CONNECTION_H_

/** @file cipclass3connection.h
 *  @brief CIP Class 3 connection
 */

#include "cipconnectionmanager.h"
#include "opener_api.h"


/**** Global variables ****/

/** @brief Array of the available explicit connections */
CIP_Connection *g_explicit_connections[OPENER_CIP_NUM_EXPLICIT_CONNS];

/** @brief Check if Class3 connection is available and if yes setup all data.
 *
 * This function can be called after all data has been parsed from the forward open request
 * @param pa_pstConnObj pointer to the connection object structure holding the parsed data from the forward open request
 * @param pa_pnExtendedError the extended error code in case an error happened
 * @return general status on the establishment
 *    - EIP_OK ... on success
 *    - On an error the general status code to be put into the response
 */
CipStatus EstablishClass3Connection(CIP_Connection* connection_object, CipUint* extended_error);

void InitializeClass3ConnectionData(void);

CIP_Connection* GetFreeExplicitConnection(void);

#endif /* OPENER_CIPCLASS3CONNECTION_H_ */
