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
#ifndef _NETSIM_QUEUE_H_
#define _NETSIM_QUEUE_H_
#ifdef  __cplusplus
extern "C" {
#endif
	typedef enum queue_type
	{
		QTYPE_P2P,
		QTYPE_APP,
	}QTYPE;

	typedef struct stru_P2P_queue
	{
		NETSIM_ID txId;
		NETSIM_ID rxId;
		NETSIM_ID txIf;
		NETSIM_ID rxIf;

		double dataRate_mbps;
		UINT64 fixedDelayQueuing_us; // IFG,...
		UINT64 fixedDelayNonQueuing_us; // PropagationDelay, ...

		UINT64 lastSentTimeQueuing;

		double ber;
	}P2PQUEUE, * ptrP2PQUEUE;

	typedef struct stru_packet_list
	{
		ptrPACKET head;
		ptrPACKET tail;

		HANDLE mutex;
		HANDLE bufferEvent;
	}PACKETLIST, * ptrPACKETLIST;

	struct stru_queue
	{
		UINT queueId;

		QTYPE type;
		ptrP2PQUEUE p2pQueue;

		PACKETLIST packetList;

		bool isQueueUsed;
		HANDLE queueEventHandler;
		DWORD queueEventHandlerId;
	};
#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_QUEUE_H_

