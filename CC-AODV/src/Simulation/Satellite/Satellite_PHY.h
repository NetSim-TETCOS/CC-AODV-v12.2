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
#ifndef _NETSIM_SATELLITE_PHY_H_
#define _NETSIM_SATELLITE_PHY_H_

#include "Satellite.h"

#ifdef  __cplusplus
extern "C" {
#endif

#include "Satellite_Frame.h"
#include "PropagationModel.h"

	typedef struct stru_satellite_phy
	{
		NETSIM_ID d;
		NETSIM_ID in;

		ptrSUPERFRAME returnLinkSuperFrame;
		ptrSUPERFRAME forwardLinkSuperFrame;

		double txPower_dbm;
		double txPower_mw;

		double txAntennaGain_db;
		double rxAntennaGain_db;
		double antennaGainToNoiseTemp_dBk;
		double tempeature_k;
	}SATELLITE_PHY, * ptrSATELLITE_PHY;
	ptrSATELLITE_PHY satellite_phy_alloc(NETSIM_ID d, NETSIM_ID in);

	typedef struct stru_satellite_ut_phy
	{
		NETSIM_ID d;
		NETSIM_ID in;

		double txPower_dbm;
		double txPower_mw;

		double txAntennaGain_db;
		double rxAntennaGain_db;
		double antennaGainToNoiseTemp_dBk;
		double tempeature_k;

		PPROPAGATION_INFO txPropagationInfo;
		PPROPAGATION_INFO rxPropagationInfo;
	}SATELLITE_UT_PHY, * ptrSATELLITE_UT_PHY;
	ptrSATELLITE_UT_PHY satellite_ut_phy_alloc(NETSIM_ID d, NETSIM_ID in);
	void satellite_ut_phy_init(NETSIM_ID d, NETSIM_ID in);

	typedef struct stru_satellite_gw_phy
	{
		NETSIM_ID d;
		NETSIM_ID in;

		double txPower_dbm;
		double txPower_mw;

		double txAntennaGain_db;
		double rxAntennaGain_db;
		double antennaGainToNoiseTemp_dBk;
		double tempeature_k;

		PPROPAGATION_INFO txPropagationInfo;
		PPROPAGATION_INFO rxPropagationInfo;
	}SATELLITE_GW_PHY, * ptrSATELLITE_GW_PHY;
	ptrSATELLITE_GW_PHY satellite_gw_phy_alloc(NETSIM_ID d, NETSIM_ID in);
	void satellite_gw_phy_init(NETSIM_ID d, NETSIM_ID in);
#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_SATELLITE_PHY_H_ */
