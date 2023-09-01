#pragma once
/************************************************************************************
 * Copyright (C) 2018
 * TETCOS, Bangalore. India															*

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 * Author:	Shashi Kant Suman														*
 * ---------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_PDCP_H_
#define _NETSIM_LTENR_PDCP_H_

#ifdef  __cplusplus
extern "C" {
#endif

	//Specification -- 38.323

#include "LTENR_Buffer.h"

#define LTENR_PDCP_SN_SIZE		12 //bits
#define LTENR_PDCP_HFN_SIZE		(32-LTENR_PDCP_SN_SIZE)
#define LTENR_PDCP_HDR_LEN		16 //Bytes
#define LTENR_PDCP_WINDOW_SIZE	0x800 //(pow(2,LTENR_PDCP_SN_SIZE-1))
#define POW_2_SN_SIZE			0xFFF
#define LTENR_PDCP_SN(state)	(state & 0x00000FFF)
#define LTENR_PDCP_HFN(state)	(state >> LTENR_PDCP_SN_SIZE)
#define LTENR_PDCP_COUNT(HFN,SN) (((HFN)<<(LTENR_PDCP_SN_SIZE))+(SN))
#define LTENR_PDCP_ISDUPLICATEPDU(entity, recv_count) ((recv_count < entity->RX_DELIV))
//#define LTENR_PDCP_ISDUPLICATEPDU(entity, recv_count) ((recv_count < entity->RX_DELIV) || (recv_count == entity->last_RECV_COUNT))
	typedef enum
	{
		DCBIT_CONTROL = 0,
		DCBIT_DATA = 1,
	}DCBIT;

	typedef struct stru_LTENR_PDCPHdr
	{
		DCBIT dcBit;
		UINT SN : LTENR_PDCP_SN_SIZE;
	}LTENR_PDCP_HDR, * ptrLTENR_PDCP_HDR;

	typedef struct stru_LTENR_PDCPEntity
	{
		NETSIM_ID ueId;
		NETSIM_ID ueIf;
		NETSIM_ID enbId;
		NETSIM_ID enbIf;
		bool isControlPacket;//true for control packet otherwise false

		//State variable. Section 7.1 of 38.323
		UINT32 TX_NEXT; /** This state variable indicates the COUNT value of the next PDCP
						SDU to be transmitted. The initial value is 0. */

		UINT32 RX_NEXT; /** This state variable indicates the COUNT value of the next PDCP
						SDU expected to be received. The initial value is 0. */

		UINT32 RX_DELIV; /** This state variable indicates the COUNT value of the
						 first PDCP SDU not delivered to the upper layers, but still waited
						 for. The initial value is 0. */

		UINT32 RX_REORD; /** This state variable indicates the COUNT value following
						 the COUNT value associated with the PDCP Data PDU which
						 triggered t-Reordering */
		UINT32 last_RECV_COUNT;

		//Buffer
		ptrLTENR_BUFFER transmissionBuffer;
		ptrLTENR_BUFFER receiveBuffer;

		//Timer
		double discardDelayTime;
		bool isDiscardTimerStarted;
		UINT64 discardTimerId;

		double  t_Reordering;
		bool isT_ReorderingRunning;
		UINT64 t_ReorderingEventId;

		struct stru_LTENR_PDCPEntity* next;
	}LTENR_PDCPENTITY, * ptrLTENR_PDCPENTITY;

	typedef struct stru_LTENR_PDCPVAR
	{
		bool isoutOfOrderDelivery;
		bool isHdrCompression;
		double discardDelayTimer;
		double t_Reordering;
		ptrLTENR_PDCPENTITY pdcpEntity;
	}LTENR_PDCPVAR, * ptrLTENR_PDCPVAR;

	typedef struct stru_LTENR_PDCP_RXBUFFER_Optional
	{
		UINT32 count;
	}LTENR_PDCPRXBUFFER_OPTIONAL, * ptrLTENR_PDCPRXBUFFER_OPTIONAL;

	void LTENR_PDCP_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
		NETSIM_ID ueId, NETSIM_ID ueIf,
		bool isAssociated);
	static ptrLTENR_PDCPENTITY LTENR_PDCPENTITY_GET(ptrLTENR_PDCPVAR pdcp,
		NETSIM_ID d,
		NETSIM_ID in,
		NETSIM_ID remote,
		bool isControlPacket);
	ptrLTENR_PDCPENTITY LTENR_PDCP_FindEntity(ptrLTENR_PDCPVAR pdcp,
		NETSIM_ID enbId,
		NETSIM_ID enbIf,
		NETSIM_ID ueId,
		NETSIM_ID ueIf,
		bool isControlPacket);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_PDCP_H_
