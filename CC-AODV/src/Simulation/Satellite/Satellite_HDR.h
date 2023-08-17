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
#ifndef _NETSIM_SATELLITE_HDR_H_
#define _NETSIM_SATELLITE_HDR_H_

#include "Satellite.h"

typedef struct stru_satellite_hdr_container
{
	UINT uniqueId;
	void* hdr;
	struct stru_satellite_hdr_container* next;
}SATELLITE_HDRCONTAINER, * ptrSATELLITE_HDRCONTAINER;

typedef struct stru_satellite_hdr
{
	NETSIM_ID utId;
	NETSIM_ID utIf;
	NETSIM_ID gwId;
	NETSIM_ID gwIf;
	NETSIM_ID satelliteId;
	NETSIM_ID satelliteIf;
	LINKTYPE linkType;

	ptrSATELLITE_HDRCONTAINER hdrContainer;
	char* name;
}SATELLITE_HDR, * ptrSATELLITE_HDR;
ptrSATELLITE_HDR SATELLITE_HDR_GET(NetSim_PACKET* packet);
void SATELLITE_HDR_REMOVE(NetSim_PACKET* packet);

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_SATELLITE_HDR_H_ */
