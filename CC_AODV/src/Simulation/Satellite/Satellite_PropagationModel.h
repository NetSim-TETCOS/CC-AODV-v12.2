#pragma once
/************************************************************************************
* Copyright (C) 2019																*
* TETCOS, Bangalore. India															*
*																					*
* Tetcos owns the intellectual property rights in the Product and its content.		*
* The copying, redistribution, reselling or publication of any or all of the		*
* Product or its content without express prior written consent of Tetcos is			*
* prohibited. Ownership and / or any other right relating to the software and all	*
* intellectual property rights therein shall remain at all times with Tetcos.		*
*																					*
* This source code is licensed per the NetSim license agreement.					*
*																					*
* No portion of this source code may be used as the basis for a derivative work,	*
* or used, for any purpose other than its intended use per the NetSim license		*
* agreement.																		*
*																					*
* This source code and the algorithms contained within it are confidential trade	*
* secrets of TETCOS and may not be used as the basis for any other software,		*
* hardware, product or service.														*
*																					*
* Author:    Shashi Kant Suman	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_SATELLITE_PROPAGATIONMODEL_H_
#define _NETSIM_SATELLITE_PROPAGATIONMODEL_H_

#include "Satellite.h"

#ifdef  __cplusplus
extern "C" {
#endif

	//Function proptotype
	void satellite_propgation_ut_init(NETSIM_ID d, NETSIM_ID in);
	void satellite_propgation_gw_init(NETSIM_ID d, NETSIM_ID in);

	void satellite_propagation_ut_calculate_rxpower(NETSIM_ID d, NETSIM_ID in, double time);
	void satellite_propagation_gw_calculate_rxpower(NETSIM_ID d, NETSIM_ID in, double time);

	bool satellite_check_for_packet_error(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri,
										  NetSim_PACKET* packet);
#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_SATELLITE_PROPAGATIONMODEL_H_ */
