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
#ifndef _NETSIM_LTENR_RLC_H_
#define _NETSIM_LTENR_RLC_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "LTENR_Buffer.h"
#include "LTENR_RLCMSG.h"
#include "List.h"

//Window Size
#if RLC_AM_SN_SIZE == 12
#define AM_Window_Size	0x800
#elif RLC_AM_SN_SIZE == 18
#define AM_Window_Size 0x20000
#endif

#if RLC_UM_SN_SIZE == 6
#define UM_Window_Size	0x20
#elif RLC_UM_SN_SIZE == 12
#define UM_Window_Size 0x800
#endif

	//Logical channel mapping
#define LTENR_RLC_IS_LOGICALCHANNEL_FOR_TM(ch)	(((ch) == LTENR_LOGICALCHANNEL_BCCH) || \
												((ch) == LTENR_LOGICALCHANNEL_CCCH) || \
												((ch) == LTENR_LOGICALCHANNEL_PCCH))

#define LTENR_RLC_IS_LOGICALCHANNEL_FOR_UM(ch)	(((ch) == LTENR_LOGICALCHANNEL_DTCH))

#define LTENR_RLC_IS_LOGICALCHANNEL_FOR_AM(ch)	(((ch) == LTENR_LOGICALCHANNEL_DCCH) || \
												((ch) == LTENR_LOGICALCHANNEL_DTCH))

	//a SN falls within the reassembly window if (RX_Next_Highest – UM_Window_Size) <= SN <RX_Next_Highest;
#define LTENR_RLC_UM_IsSNFallsInReassemblyWindows(entity,sn) ((((entity)->UM.RX_Next_Highest - UM_Window_Size) <= (sn)) && ((sn)<(entity)->UM.RX_Next_Highest))

	typedef struct stru_LTENR_RLC_ReceptionBuffer
	{
		UINT reassembleCount;
		UINT SN;
		UINT lastSDUSO;
		bool isLastSDUReceived;
		NetSim_PACKET* head;
		NetSim_PACKET* tail;
		_ptr_ele ele;
	}LTENR_RLC_RECEPTIONBUFFER, *ptrLTENR_RLC_RECEPTIONBUFFER;
#define LTENR_RLC_RECEPTIONBUFFER_ALLOC() ((ptrLTENR_RLC_RECEPTIONBUFFER)list_alloc(sizeof(LTENR_RLC_RECEPTIONBUFFER),offsetof(LTENR_RLC_RECEPTIONBUFFER,ele)))
#define LTENR_RLC_RECEPTIONBUFFER_NEXT(buf) ((ptrLTENR_RLC_RECEPTIONBUFFER)LIST_NEXT(buf))
#define LTENR_RLC_RECEPTIONBUFFER_ADD(ls,mem) (LIST_ADD_LAST(ls,mem))

	typedef struct stru_LTENR_RLCBuffer
	{
		SI si;
		UINT SN;
		UINT16 SO;

		UINT retxCount;
		LTENR_LOGICALCHANNEL channel;
		NetSim_PACKET* packet;
		bool isMarkedForRemoval;
		bool isFragmented;
		_ptr_ele ele;
	}LTENR_RLCBUFFER, *ptrLTENR_RLCBUFFER;
#define LTENR_RLCBUFFER_ALLOC() ((ptrLTENR_RLCBUFFER)list_alloc(sizeof(LTENR_RLCBUFFER),offsetof(LTENR_RLCBUFFER,ele)))
#define LTENR_RLCBUFFER_NEXT(buf) (buf = (ptrLTENR_RLCBUFFER)LIST_NEXT(buf))
#define LTENR_RLCBUFFER_ADD(ls,mem) (LIST_ADD_LAST((void**)ls,mem))
#define LTENR_RLCBUFFER_REMOVE(ptrbuffer, mem) (LIST_REMOVE((void**)(ptrbuffer),(mem)))
#define LTENR_RLCBuffer_MarkForRemoval(buf)	((buf)->isMarkedForRemoval = true)

	ptrLTENR_RLCBUFFER LTENR_RLCBuffer_AddPacket(ptrLTENR_RLCBUFFER* buffer, NetSim_PACKET* packet,
												 UINT sn, UINT16 so, LTENR_LOGICALCHANNEL channel);
	void LTENR_RLCBuffer_RemovePacket(ptrLTENR_RLCBUFFER* buffer, ptrLTENR_RLCBUFFER mem);
	void LTENR_RLCBuffer_FreePacket(ptrLTENR_RLCBUFFER* buffer, ptrLTENR_RLCBUFFER mem);
	void LTENR_RLCBuffer_UpdateBuffer(ptrLTENR_RLCBUFFER* buffer);
	ptrLTENR_RLCBUFFER LTENR_RLCBUFFER_FindAndRemoveAllBySN(ptrLTENR_RLCBUFFER* buffer, UINT sn);
	ptrLTENR_RLCBUFFER LTENR_RLCBUFFER_FindAndRemoveBySNSO(ptrLTENR_RLCBUFFER* buffer, UINT sn, UINT16 so);
 
	typedef struct stru_LTENR_RLCAM_Status
	{
		SI si;
		UINT SN;
		UINT16 SO;
		_ptr_ele ele;
	}LTENR_RLCAM_STATUS, *ptrLTENR_RLCAM_STATUS;
#define LTENR_RLCAM_STATUS_ALLOC() ((ptrLTENR_RLCAM_STATUS)list_alloc(sizeof(LTENR_RLCAM_STATUS),offsetof(LTENR_RLCAM_STATUS,ele)))
#define LTENR_RLCAM_STATUS_NEXT(status) (status=(ptrLTENR_RLCAM_STATUS)LIST_NEXT(status))

	typedef struct stru_LTENR_RLCEntity
	{
		NETSIM_ID d;
		NETSIM_ID in;
		NETSIM_ID peerId;
		NETSIM_ID peerIf;

		LTENR_RLCMODE rlcMode;
		bool isTransmittingRLCEntity;
		bool isReceivingRLCEntity;

		struct stru_LTENR_RLCAM_StateVariables_Constants
		{
			//RLC-AM Transmitting side state variable
			UINT TX_Next_Ack;
			UINT TX_Next;
			UINT POLL_SN;
			UINT TX_HighestSN;

			//RLC-AM Transmitting side counter
			UINT PDU_WITHOUT_POLL;
			UINT BYTE_WITHOUT_POLL;
			UINT RETX_COUNT;

			//RLC-AM Receiving side state variable
			UINT RX_Next;
			UINT RX_Next_Status_Trigger;
			UINT RX_Highest_Status;
			UINT RX_Next_Highest;

			ptrLTENR_RLC_RECEPTIONBUFFER receptionBuffer;
			bool isStatusReportDelayed;
			bool isConstructStatusReortDelayed;
			ptrLTENR_RLCAM_STATUS recvdInfo;
		}AM;

		struct stru_LTENR_RLCUM_StateVariables_Constants
		{
			//RLC-UM Transmitting side state vriables and counters
			UINT TX_Next;

			//RLC-UM Receiving side state vriables and counters
			UINT RX_Timer_Trigger;
			UINT RX_Next_Reassembly;
			UINT RX_Next_Highest;

			ptrLTENR_RLC_RECEPTIONBUFFER receptionBuffer;
		}UM;

		//Timer
		double t_PollRetransmit;
		bool isPollRetransmitTimerStarted;
		UINT64 pollRetransmitTimerId;

		double t_Reassembly;
		bool isReassemblyTimerStarted;
		UINT64 reassemblyTimerId;

		double t_StatusProhibit;
		bool isStatusProhibitTimerStarted;
		UINT64 statusProhibitTimerId;

		//Config parameter
		UINT maxRetxThreshold;
		UINT pollPDU;
		UINT pollByte;

		//Buffer
		ptrLTENR_RLCBUFFER transmissionBuffer;
		ptrLTENR_RLCBUFFER transmittedBuffer;
		ptrLTENR_RLCBUFFER retransmissionBuffer;
		UINT bufferStatus[LTENR_LOGICALCHANNEL_LAST];

		_ptr_ele ele;
	}LTENR_RLCENTITY, *ptrLTENR_RLCENTITY;
#define LTENR_RLCENTITY_ALLOC()			(list_alloc(sizeof(LTENR_RLCENTITY),offsetof(LTENR_RLCENTITY,ele)))
#define LTENR_RLCENTITY_NEXT(entity)	(LIST_NEXT(entity))
#define LTENR_RLCENTITY_ADD(rlc,e)		(LIST_ADD_LAST(&((rlc)->rlcEntities),e))
#define LTENR_RLCENTITY_REMOVE(rlc,e)	(LIST_REMOVE(&((rlc)->rlcEntities),e)) 

	typedef struct stru_LTENR_RLCVar
	{
		double t_reassembly;
		ptrLTENR_RLCENTITY rlcEntities;
	}LTENR_RLCVAR, *ptrLTENR_RLCVAR;
#define LTENR_RLC_GET_TM_TRANSMITTING_ENTITY(d,in,r,rin)	(LTENR_RLC_FINDENTITY(d,in,r,rin,LTENR_RLCMODE_TM,true,false))
#define LTENR_RLC_GET_TM_RECEIVING_ENTITY(d,in,r,rin)		(LTENR_RLC_FINDENTITY(d,in,r,rin,LTENR_RLCMODE_TM,false,true))
#define LTENR_RLC_GET_UM_TRANSMITTING_ENTITY(d,in,r,rin)	(LTENR_RLC_FINDENTITY(d,in,r,rin,LTENR_RLCMODE_UM,true,false))
#define LTENR_RLC_GET_UM_RECEIVING_ENTITY(d,in,r,rin)		(LTENR_RLC_FINDENTITY(d,in,r,rin,LTENR_RLCMODE_UM,false,true))
#define LTENR_RLC_GET_AM_TRANSMITTING_ENTITY(d,in,r,rin)	(LTENR_RLC_FINDENTITY(d,in,r,rin,LTENR_RLCMODE_AM,true,true))
#define LTENR_RLC_GET_AM_RECEIVING_ENTITY(d,in,r,rin)		(LTENR_RLC_FINDENTITY(d,in,r,rin,LTENR_RLCMODE_AM,true,true))


	//Function prototype
	//RLC TM
	void LTENR_RLC_TM_HandlePDUFromUpperLayer();

	//RLC UM
	void LTENR_RLC_UM_HandlePDUFromUpperLayer();
	UINT LTENR_RLC_UM_SEND_PDU(NETSIM_ID d, NETSIM_ID in,
							   NETSIM_ID r, NETSIM_ID rin,
							   UINT size, LTENR_LOGICALCHANNEL logicalChannel);
	void LTENR_RLC_UM_RECEIVE_PDU();
	void LTENR_RLC_UM_Handle_t_reassemblyTimer();

	//RLC AM
	void LTENR_RLCAM_HandlePDUFromUpperLayer();
	UINT LTENR_RLCAM_SEND_PDU(NETSIM_ID d, NETSIM_ID in,
							  NETSIM_ID r, NETSIM_ID rin,
							  UINT size, LTENR_LOGICALCHANNEL logicalChannel);
	void LTENR_RLC_AM_RECEIVE_PDU();
	void LTENR_RLCAM_Start_t_ReassemblyTimer(ptrLTENR_RLCENTITY entity);
	void LTENR_RLCAM_Handle_t_reassemblyTimer();
	void LTENR_RLCAM_Handle_t_PollRetransmitTimer();
	void LTENR_RLCAM_Handle_t_StatusProhibitTimer();
	void LTENR_RLCAM_ReceiveStatusPDU();

	//RLC Entity
	ptrLTENR_RLCENTITY LTENR_RLC_FINDENTITY(NETSIM_ID d, NETSIM_ID in,
											NETSIM_ID peerId, NETSIM_ID peerIf,
											LTENR_RLCMODE mode,
											bool isTransmitting,
											bool isReceiving);
	void LTENR_RLC_ESTABLISHENTITYALL(NETSIM_ID d, NETSIM_ID in,
		NETSIM_ID peerId, NETSIM_ID peerIf);
	void LTENR_RLC_TMESTABLISHENTITYALL(NETSIM_ID d, NETSIM_ID in,
		NETSIM_ID peerId, NETSIM_ID peerIf);
	void LTENR_RLC_ASSOCIATION(NETSIM_ID d, NETSIM_ID in,
		NETSIM_ID peerId, NETSIM_ID peerIf,
		bool isAssociated);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_RLC_H_
