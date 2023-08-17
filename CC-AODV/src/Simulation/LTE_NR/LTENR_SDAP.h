#pragma once
/************************************************************************************
 * Copyright (C) 2019
 * TETCOS, Bangalore. India															*

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 * Author:	Shashi Kant Suman														*
 * ---------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_SDAP_H_
#define _NETSIM_LTENR_SDAP_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "List.h"
	typedef struct stru_LTENR_SDAP_DelayEntity {
		UINT appID;
		UINT64 packetID;
		UINT segmentID;
		double TxTime;
		double RxTime;
		double delay;
		_ptr_ele ele;
	}LTENR_SDAP_DelayEntity, * ptrLTENR_SDAP_DelayEntity;
#define LTENR_SDAP_DELAYENTITY_ALLOC()			(list_alloc(sizeof(LTENR_SDAP_DelayEntity),offsetof(LTENR_SDAP_DelayEntity,ele)))
#define LTENR_SDAP_DELAYENTITY_NEXT(entity)		(LIST_NEXT(entity))
#define LTENR_SDAP_DELAYENTITY_ADD(entity,e)	(LIST_ADD_LAST(&((entity)->delayList),e))
#define LTENR_SDAP_DELAYENTITY_REMOVE(entity,e)	(LIST_FREE(&((entity)->delayList),e))

	typedef struct stru_LTENR_SDAP_Entity
	{
		NETSIM_ID entityID;
		NETSIM_ID d;
		NETSIM_ID in;
		NETSIM_ID r;
		NETSIM_ID rin;
		NETSIM_IPAddress srcIP;
		NETSIM_ID srcPort;
		NETSIM_IPAddress desIP;
		NETSIM_ID desPort;
		UINT countIn;
		UINT countOut;
		UINT QFI;
		UINT appID;
		ptrLTENR_SDAP_DelayEntity delayList;
		_ptr_ele ele;
	}LTENR_SDAPENTITY, * ptrLTENR_SDAPENTITY;
#define LTENR_SDAPENTITY_ALLOC()			(list_alloc(sizeof(LTENR_SDAPENTITY),offsetof(LTENR_SDAPENTITY,ele)))
#define LTENR_SDAPNTITY_NEXT(entity)		(LIST_NEXT(entity))
#define LTENR_SDAPENTITY_ADD(sdap,e)		(LIST_ADD_LAST(&((sdap)->sdapEntities),e))
#define LTENR_SDAPENTITY_REMOVE(sdap,e)		(LIST_FREE(&((sdap)->sdapEntities),e)) 

#define LTENR_SDAP_AVGWINDOW 		2000*MILLISECOND
#define LTENR_SDAP_Transmission_Threshold 50*MILLISECOND
#define LTENR_SDAP_DefaultPDB 100*MILLISECOND
	unsigned int LTENR_SDAP_ENTITYCOUNT = 1;
	unsigned int LTENR_SDAP_QFICOUNT = 1;

	typedef struct stru_LTENR_SDAP_Var
	{
		ptrLTENR_SDAPENTITY sdapEntities;
	}LTENR_SDAPVAR, * ptrLTENR_SDAPVAR;

	typedef struct stru_LTENR_SDAP_DLHDR
	{
		UINT RDI : 1;
		UINT RQI : 1;
		UINT QFI : 6;
	}LTENR_SDAP_DLHDR, * ptrLTENR_SDAP_DLHDR;

	typedef struct stru_LTENR_SDAP_ULHDR
	{
		UINT DCBit : 1;
		UINT R : 1;
		UINT QFI : 6;
	}LTENR_SDAP_ULHDR, * ptrLTENR_SDAP_ULHDR;
#define LTENR_SDAP_HDR_LEN	1 //bytes
#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_SDAP_H_
