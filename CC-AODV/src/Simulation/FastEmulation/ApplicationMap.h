#pragma once
/************************************************************************************
* Copyright (C) 2020																*
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
#ifndef _NETSIM_APPLICATIONMAP_H_
#define _NETSIM_APPLICATIONMAP_H_
#ifdef  __cplusplus
extern "C" {
#endif

	struct stru_app_map
	{
		NETSIM_ID appId;
		
		bool isMultipleInstance;
		NETSIM_ID instanceId;
		UINT instanceCount;
		HANDLE mutex;
		UINT64 appStartTime;
		UINT64 appEndTime;
		bool isAppStarted;

		NETSIM_IPAddress srcIP;
		NETSIM_IPAddress destIP;
		UINT16 srcPort;
		UINT16 destPort;
		char* protocol;
		char* filterString;

		NETSIM_ID simSrcId;
		NETSIM_ID simDestId;

		//Path
		UINT hopCount;
		ptrQUEUE* hops;

		bool isNonInterferingApp;
		double queuingRate;
		double nonQueuingRate;
		UINT64 totalFixedDelay;

		//ber
		double ber;
		

		void* headPacket;
		void* tailPacket;
		HANDLE queueMutex;

		UINT64 lastReinjectTime;
	};

	ptrAPPMAP fastemulation_init_new_instance_for_app(ptrAPPMAP map);

#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_APPLICATIONMAP_H_
