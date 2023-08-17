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
#ifndef _NETSIM_SATELLITE_MAC_H_
#define _NETSIM_SATELLITE_MAC_H_

#include "Satellite.h"

#ifdef  __cplusplus
extern "C" {
#endif

#include "Satellite_Buffer.h"
#include "Satellite_Frame.h"

	typedef struct stru_Satellite_UT_Mac
	{
		NETSIM_ID utId;
		NETSIM_ID utIf;

		char* szGatewayName;
		NETSIM_ID gatewayId;
		NETSIM_ID gatewayIf;

		char* satelliteName;
		NETSIM_ID satelliteId;
		NETSIM_ID satelliteIf;

		UINT bufferSize_bits;
		ptrSATELLITE_BUFFER buffer;
	}SATELLITE_UT_MAC, * ptrSATELLITE_UT_MAC;
	ptrSATELLITE_UT_MAC satellite_ut_mac_alloc(ptrSATELLITE_PROTODATA pd);

	typedef struct stru_satellite_ut_assoc_info
	{
		NETSIM_ID utId;
		NETSIM_ID utIf;
		ptrSATELLITE_BUFFER buffer;
	}SATELLITE_UTASSOCINFO, * ptrSATELLITE_UTASSOCINFO;
	void satellite_assoc_ut(NETSIM_ID gwId, NETSIM_ID gwIf,
							NETSIM_ID utId, NETSIM_ID utIf);
	ptrSATELLITE_UTASSOCINFO satellite_utassocinfo_find(NETSIM_ID gwId, NETSIM_ID gwIf,
														NETSIM_ID utId, NETSIM_ID utIf);

	typedef struct stru_satellite_GW_Mac
	{
		NETSIM_ID gwId;
		NETSIM_ID gwIf;

		UINT bufferSize_bits;

		char* satelliteName;
		NETSIM_ID satelliteId;
		NETSIM_ID satelliteIf;

		UINT nAssocUTCount;
		ptrSATELLITE_UTASSOCINFO* utAssocInfo;
	}SATELLITE_GW_MAC, * ptrSATELLITE_GW_MAC;
	ptrSATELLITE_GW_MAC satellite_gw_mac_alloc(ptrSATELLITE_PROTODATA pd);

	typedef struct stru_satellite_mac
	{
		NETSIM_ID satelliteId;
		NETSIM_ID satelliteIf;

		ptrSUPERFRAME returnLinkSuperFrame;
		ptrSUPERFRAME forwardFLinkSuperFrame;
	}SATELLITE_MAC, * ptrSATELLITE_MAC;
	ptrSATELLITE_MAC satellite_mac_alloc(ptrSATELLITE_PROTODATA pd);
	void satellite_GW_MAC_init(NETSIM_ID gwId, NETSIM_ID gwIf);

	//Function prototype
	void satellite_ut_set_gateway(ptrSATELLITE_UT_MAC utMac);
	void satellite_UT_MAC_init(NETSIM_ID utId, NETSIM_ID utIf);
	void satellite_mac_init(NETSIM_ID d, NETSIM_ID in);
	void satellite_form_bufferList(NETSIM_ID d, NETSIM_ID in,
								   ptrSUPERFRAME sf);
#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_SATELLITE_MAC_H_ */
