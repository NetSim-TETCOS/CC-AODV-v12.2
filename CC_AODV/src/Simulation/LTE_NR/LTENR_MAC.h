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
#ifndef _NETSIM_LTENR_MAC_H_
#define _NETSIM_LTENR_MAC_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "LTENR_PHY.h"

	typedef enum enum_LTENR_MAC_GnbSchedulingType {
		LTENR_MAC_SCHEDULING_ROUNDROBIN,
		LTENR_MAC_SCHEDULING_PROPORTIONALFAIR,
		LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT,
		LTENR_MAC_SCHEDULING_FAIR_SCHEDULING,
	}LTENR_GnbSchedulingType;


	typedef struct stru_LTENR_UESchedularInfo
	{
		__IN__ NETSIM_ID ueId;
		__IN__ NETSIM_ID ueIf;
		__IN__ UINT bufferSize;
		__IN__ UINT cqiIndex;
		__IN__ UINT64 bitsPerPRB;
		__IN__ double rank;
		__IN__ double initRank;

		__OUT__ UINT allocatedPRBCount;
		struct stru_LTENR_UESchedularInfo* next;
	}LTENR_UESCHEDULARINFO, * ptrLTENR_UESCHEDULARINFO;

	typedef struct stru_LTENR_SchedularInfo
	{
		__IN__ NETSIM_ID gnbId;
		__IN__ NETSIM_ID gnbIf;

		__IN__ UINT PRBCount;
		__IN__ double OH_UL;
		__IN__ double OH_DL;
		__IN__ BOOL isPRBRankInit;
		__IN__ LTENR_GnbSchedulingType schedulingType;
		__IN__ __OUT__ LTENR_SLOTTYPE slotType; 

		__IN__ ptrLTENR_UESCHEDULARINFO uplinkInfo;
		__IN__ ptrLTENR_UESCHEDULARINFO downlinkInfo;
	}LTENR_SCHEDULARINFO, * ptrLTENR_SCHEDULARINFO;

	typedef struct stru_LTENR_GnbMac
	{
		NETSIM_ID gnbId;
		NETSIM_ID gnbIf;

		NETSIM_ID epcId;
		NETSIM_ID epcIf;

		ptrLTENR_SCHEDULARINFO schedularInfo[MAX_CA_COUNT];
	}LTENR_GNBMAC, * ptrLTENR_GNBMAC;


	double initTotalPRBAvailable;
	void LTENR_PRB_Schedular(ptrLTENR_SCHEDULARINFO schedularInfo);

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_MAC_H_ */