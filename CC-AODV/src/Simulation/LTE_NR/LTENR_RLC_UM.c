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

#include "stdafx.h"
#include "LTENR_RLC.h"
#include "LTENR_RLCMSG.h"

#pragma region RLC_UM_RECEPTIONBUFFER
static ptrLTENR_RLC_RECEPTIONBUFFER LTENR_RLC_UM_FINDRECEPTIONBUFFER(ptrLTENR_RLCENTITY entity,
																	 UINT sn)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buf = entity->UM.receptionBuffer;
	while (buf)
	{
		if (buf->SN == sn)
			return buf;
		buf = LTENR_RLC_RECEPTIONBUFFER_NEXT(buf);
	}
	return NULL;
}

static ptrLTENR_RLC_RECEPTIONBUFFER LTENR_RLC_UM_CREATERECEPTIONBUFFER(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buf = LTENR_RLC_RECEPTIONBUFFER_ALLOC();
	buf->SN = sn;
	LTENR_RLC_RECEPTIONBUFFER_ADD(&entity->UM.receptionBuffer, buf);
	return buf;
}

static void LTENR_RLC_UM_DeleteReceptionBuffer(ptrLTENR_RLCENTITY entity,
											   ptrLTENR_RLC_RECEPTIONBUFFER buffer)
{
	LIST_REMOVE(&entity->UM.receptionBuffer, buffer);
}

static bool LTENR_RLC_UM_isAllByteSegmentReceived(ptrLTENR_RLC_RECEPTIONBUFFER buffer)
{
	NetSim_PACKET* p = buffer->head;
	if (!buffer->isLastSDUReceived)
		return false;
	bool* isRecv;
	isRecv = calloc((size_t)buffer->lastSDUSO + 1, sizeof* isRecv);

	while (p)
	{
		ptrLTENR_RLCMSG_UMDHDR h = LTENR_RLCMSG_UMHDR_GET(p);
		assert(h->SO <= buffer->lastSDUSO);
		isRecv[h->SO] = true;
		p = p->pstruNextPacket;
	}

	bool isAllRecv = true;
	UINT16 i;
	for (i = 0; i < buffer->lastSDUSO; i++)
	{
		if (!isRecv[i])
		{
			isAllRecv = false;
			break;
		}
	}
	free(isRecv);
	return isAllRecv;
}

static bool LTENR_RLC_UM_isAllBytesReceivedForSN(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = LTENR_RLC_UM_FINDRECEPTIONBUFFER(entity, sn);
	if (!buffer)
		return true;
	return LTENR_RLC_UM_isAllByteSegmentReceived(buffer);
}

static void LTENR_RLC_UM_AddToReceptionBuffer(ptrLTENR_RLC_RECEPTIONBUFFER buffer,
											  NetSim_PACKET* packet)
{
	if (buffer->head)
	{
		buffer->tail->pstruNextPacket = packet;
		buffer->tail = packet;
	}
	else
	{
		buffer->head = packet;
		buffer->tail = packet;
	}

	ptrLTENR_RLCMSG_UMDHDR h = LTENR_RLCMSG_UMHDR_GET(packet);
	if (h->si == SI_LAST)
	{
		buffer->isLastSDUReceived = true;
		buffer->lastSDUSO = h->SO;
	}
}

static NetSim_PACKET* LTENR_RLC_UM_ReassembleAllBytes(ptrLTENR_RLC_RECEPTIONBUFFER buffer)
{
	NetSim_PACKET* packet = buffer->head;
	NetSim_PACKET* loop = buffer->head->pstruNextPacket;
	NetSim_PACKET* t;
	while (loop)
	{
		packet->pstruMacData->dPayload += loop->pstruMacData->dPayload;
		packet->pstruMacData->dPacketSize += loop->pstruMacData->dPayload;
		t = loop;
		loop = loop->pstruNextPacket;
		fn_NetSim_Packet_FreePacket(t);
	}
	packet->pstruNextPacket = NULL;
	return packet;
}

static void LTENR_RLC_UM_UpdateRXNextReassembly(ptrLTENR_RLCENTITY entity)
{
	UINT x = entity->UM.RX_Next_Reassembly;
	ptrLTENR_RLC_RECEPTIONBUFFER buf = entity->UM.receptionBuffer;
	while (buf)
	{
		if (buf->SN > x)
		{
			entity->UM.RX_Next_Reassembly = buf->SN;
			break;
		}
		buf = LIST_NEXT(buf);
	}
}

static void LTENR_RLC_UM_PLACE_UMDPDU_IN_RECEPTIONBUFFER(ptrLTENR_RLCENTITY entity,
														 NetSim_PACKET* packet)
{
	ptrLTENR_RLCMSG_UMDHDR hdr = LTENR_RLCMSG_UMHDR_GET(packet);
	UINT x = hdr->SN;

	ptrLTENR_RLC_RECEPTIONBUFFER buffer = LTENR_RLC_UM_FINDRECEPTIONBUFFER(entity, hdr->SN);
	if (!buffer)
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("No recption buffer found for SN %d. Creating new buffer.\n", hdr->SN);
#endif
		buffer = LTENR_RLC_UM_CREATERECEPTIONBUFFER(entity, hdr->SN);
	}
	LTENR_RLC_UM_AddToReceptionBuffer(buffer, packet);


	if (LTENR_RLC_UM_isAllByteSegmentReceived(buffer))
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("All bytes for SDU with SN %d received.\n"
						"Reassembly all bytes.\n", hdr->SN);
#endif
		packet = LTENR_RLC_UM_ReassembleAllBytes(buffer);
		LTENR_RLCMSG_UMDHDR_REMOVE(packet);
		LTENR_RLC_UM_DeleteReceptionBuffer(entity, buffer);
		buffer = NULL;
#ifdef LTENR_RLC_LOG
		print_ltenr_log("SDU payload size = %d bytes\n"
						"SDU overhead size = %d bytes.\n"
						"SDU total size = %d bytes."
						"Sending SDU to PDCP.\n", (UINT)packet->pstruMacData->dPayload,
						(UINT)packet->pstruMacData->dOverhead, (UINT)packet->pstruMacData->dPacketSize);
#endif
		pstruEventDetails->pPacket = packet;
		LTENR_CallPDCPIN();

		if (x == entity->UM.RX_Next_Reassembly)
			LTENR_RLC_UM_UpdateRXNextReassembly(entity);
	}
}

static UINT LTENR_RLC_UM_ReceptionBuffer_FindSN(ptrLTENR_RLCENTITY entity, UINT sn, char* op)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = entity->UM.receptionBuffer;
	while (buffer)
	{
		if (CompareVal(buffer->SN, sn, op))
			return buffer->SN;
		buffer = LIST_NEXT(buffer);
	}
	return 0;
}

static void LTENR_RLC_UN_DiscardAllSengmentFromReceptionBuffer(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = entity->UM.receptionBuffer;
	while (buffer)
	{
		if (buffer->SN < sn)
		{
			NetSim_PACKET* p = buffer->head;
			NetSim_PACKET* t;
			while (p)
			{
				t = p;
				p = p->pstruNextPacket;
				fn_NetSim_Packet_FreePacket(t);
			}
			buffer->head = NULL;
			buffer->tail = NULL;
			LIST_FREE(&entity->UM.receptionBuffer, buffer);
			buffer = entity->UM.receptionBuffer;
			continue;
		}
		buffer = LIST_NEXT(buffer);
	}
}
#pragma endregion

#pragma region RLC_UM_t_Reassembly_timer
void LTENR_RLC_UM_Start_t_ReassemblyTimer(ptrLTENR_RLCENTITY entity)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime + entity->t_Reassembly;
	pevent.nDeviceId = entity->d;
	pevent.nDeviceType = DEVICE_TYPE(entity->d);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = entity->in;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(entity->d, entity->in);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVNT_RLC_TREASSEMBLYTIMER);
	pevent.szOtherDetails = entity;
	entity->reassemblyTimerId = fnpAddEvent(&pevent);
	entity->isReassemblyTimerStarted = true;
}

void LTENR_RLC_UM_Stop_t_ReassemblyTimer(ptrLTENR_RLCENTITY entity)
{
	if (!entity->isReassemblyTimerStarted)
		return;
	fnDeleteEvent(entity->reassemblyTimerId);
	entity->isReassemblyTimerStarted = false;
}

void LTENR_RLC_UM_Handle_t_reassemblyTimer()
{
	ptrLTENR_RLCENTITY entity = pstruEventDetails->szOtherDetails;

	entity->UM.RX_Next_Reassembly = LTENR_RLC_UM_ReceptionBuffer_FindSN(entity, entity->UM.RX_Timer_Trigger, ">=");
	LTENR_RLC_UN_DiscardAllSengmentFromReceptionBuffer(entity, entity->UM.RX_Next_Reassembly);

	if (entity->UM.RX_Next_Highest > entity->UM.RX_Next_Reassembly + 1 ||
		(entity->UM.RX_Next_Highest == entity->UM.RX_Next_Reassembly + 1 &&
		 !LTENR_RLC_UM_isAllBytesReceivedForSN(entity, entity->UM.RX_Next_Reassembly)))
	{
		LTENR_RLC_UM_Start_t_ReassemblyTimer(entity);
		entity->UM.RX_Timer_Trigger = entity->UM.RX_Next_Highest;
	}
}
#pragma endregion

#pragma region RLC_UM_OUT
void LTENR_RLC_UM_HandlePDUFromUpperLayer()
{
	print_ltenr_log("Packet is for UM mode.\n");
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, packet->nReceiverId);
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_UM_TRANSMITTING_ENTITY(d, in,
																	 packet->nReceiverId, rin);

	print_ltenr_log("Adding packet to transmission buffer.\n");
	LTENR_LOGICALCHANNEL channel = LTENR_PACKET_GET_LOGICALCHANNEL(packet);
	LTENR_RLCBuffer_AddPacket(&entity->transmissionBuffer, packet, 0, 0, channel);
	entity->bufferStatus[channel] += (UINT)packet->pstruMacData->dPayload;
}

static NetSim_PACKET* LTENR_RLC_UM_FRAGMENT_PACKET(NetSim_PACKET* p, UINT size, UINT sn)
{
	ptrLTENR_RLCMSG_UMDHDR hdr = LTENR_RLCMSG_UMHDR_GET(p);
	UINT16 so;
	SI si;
	if (hdr)
	{
		so = hdr->SO;
		si = SI_MIDDLE;
	}
	else
	{
		so = 0;
		si = SI_FIRST;
	}

	LTENR_RLCMSG_UMDHDR_REMOVE(p);

	NetSim_PACKET* ret = fn_NetSim_Packet_CopyPacket(p);
	ret->pstruMacData->dPayload = size;
	ret->pstruMacData->dPacketSize = ret->pstruMacData->dPayload + ret->pstruMacData->dOverhead;

	p->pstruMacData->dPayload -= size;
	p->pstruMacData->dPacketSize -= size;

	LTENR_RLCMSG_UMHDR_ADD(ret, si, sn, so);
	LTENR_RLCMSG_UMHDR_ADD(p, SI_MIDDLE, sn, so + 1);
	return ret;
}

static int LTENR_RLC_UM_ADD_HDR(NetSim_PACKET* p)
{
	int ret = -1;
	UINT size;
	ptrLTENR_RLCMSG_UMDHDR hdr = LTENR_RLCMSG_UMHDR_GET(p);
	UINT16 so;
	SI si;
	UINT sn;
	if (hdr)
	{
		sn = hdr->SN;
		so = hdr->SO;
		si = SI_LAST;
		ret = sn;
	}
	else
	{
		sn = 0;
		so = 0;
		si = SI_ALL;
		ret = -1;
	}

	LTENR_RLCMSG_UMDHDR_REMOVE(p);
	size = LTENR_RLCMSG_UMHDR_ADD(p, si, sn, so);
	return ret;
}

UINT LTENR_RLC_UM_SEND_PDU(NETSIM_ID d, NETSIM_ID in,
						   NETSIM_ID r, NETSIM_ID rin,
						   UINT size, LTENR_LOGICALCHANNEL logicalChannel)
{
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_UM_TRANSMITTING_ENTITY(d, in, r, rin);
	NetSim_PACKET* umdpdu = NULL;

	UINT left = size;
	if (entity) {
		ptrLTENR_RLCBUFFER buf = entity->transmissionBuffer;
		while (buf)
		{
			umdpdu = buf->packet;

			if (buf->channel != logicalChannel)
			{
				LTENR_RLCBUFFER_NEXT(buf);
				continue;
			}

			if (left < MIN_RLC_PDU_SIZE)
				break;

			if (LTENR_IS_UPLANE_MSG(umdpdu) &&
				umdpdu->pstruMacData->dPayload > left)
			{
				UINT sn = entity->UM.TX_Next;
				umdpdu = LTENR_RLC_UM_FRAGMENT_PACKET(umdpdu, left, sn);
			}
			else
			{
				int sn = LTENR_RLC_UM_ADD_HDR(umdpdu);
				if (sn != -1)
					entity->UM.TX_Next++;
				LTENR_RLCBuffer_MarkForRemoval(buf);
				LTENR_RLCBUFFER_NEXT(buf);
			}

			left -= (UINT)umdpdu->pstruMacData->dPayload;
			entity->bufferStatus[logicalChannel] -= (UINT)umdpdu->pstruMacData->dPayload;
			LTENR_RLC_SENDPDUTOMAC(d, in, umdpdu);
		}
		LTENR_RLCBuffer_UpdateBuffer(&entity->transmissionBuffer);
		return size - left;
	}
}
#pragma endregion

#pragma region RLC_UM_IN
static UINT LTENR_RLC_UM_NextRXReassembly(ptrLTENR_RLCENTITY entity)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buf = entity->UM.receptionBuffer;
	while (buf)
	{
		if (buf->SN >= (entity->UM.RX_Next_Highest - UM_Window_Size))
		{
			return buf->SN;
		}
	}
	return 0;
}

static void LTENR_RLC_UM_RECEIVE_PDU_WITH_SN()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID r = packet->nTransmitterId;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, r);
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_UM_RECEIVING_ENTITY(d, in, r, rin);
	if (!entity)
		return;
	ptrLTENR_RLCMSG_UMDHDR hdr = LTENR_RLCMSG_UMHDR_GET(packet);
	UINT x = hdr->SN;

	if (LTENR_RLC_UM_IsSNFallsInReassemblyWindows(entity,x))
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("UMD PDU falls outside of reassembly windows.\n"
						"Dropping UMD PDU.\n");
#endif
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;

		entity->UM.RX_Next_Highest = x + 1;

		if (LTENR_RLC_UM_IsSNFallsInReassemblyWindows(entity, entity->UM.RX_Next_Reassembly))
			entity->UM.RX_Next_Reassembly = LTENR_RLC_UM_NextRXReassembly(entity);
	}
	else
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("Placing SDU to reception buffer\n");
#endif
		LTENR_RLC_UM_PLACE_UMDPDU_IN_RECEPTIONBUFFER(entity, packet);
	}

	if (entity->isReassemblyTimerStarted)
	{
		if ((entity->UM.RX_Timer_Trigger <= entity->UM.RX_Next_Reassembly) ||
			((entity->UM.RX_Next_Highest - UM_Window_Size <= entity->UM.RX_Timer_Trigger) &&
			 (entity->UM.RX_Timer_Trigger != entity->UM.RX_Next_Highest)) ||
			((entity->UM.RX_Next_Highest == entity->UM.RX_Next_Reassembly+1)&&
			 LTENR_RLC_UM_isAllBytesReceivedForSN(entity,entity->UM.RX_Next_Reassembly)))
		{
#ifdef LTENR_RLC_LOG
			print_ltenr_log("Reseting t_reassembly timer.\n");
#endif // LTENR_RLC_LOG

			LTENR_RLC_UM_Stop_t_ReassemblyTimer(entity);
			LTENR_RLC_UM_Start_t_ReassemblyTimer(entity);
		}
	}
	else
	{
		if ((entity->UM.RX_Next_Highest > entity->UM.RX_Next_Reassembly + 1) ||
			((entity->UM.RX_Next_Highest == entity->UM.RX_Next_Reassembly + 1) &&
			(!LTENR_RLC_UM_isAllBytesReceivedForSN(entity, entity->UM.RX_Next_Reassembly))))
		{
#ifdef LTENR_RLC_LOG
			print_ltenr_log("Starting t_reassembly timer.\n");
#endif // LTENR_RLC_LOG
			LTENR_RLC_UM_Start_t_ReassemblyTimer(entity);
			entity->UM.RX_Timer_Trigger = entity->UM.RX_Next_Highest;
		}
	}

#ifdef LTENR_RLC_LOG
	print_ltenr_log("RLC state variable...\n"
					"\t RX_Next_Highest = %d\n"
					"\t RX_Next_Reassembly = %d\n"
					"\t RX_Timer_Trigger = %d\n",
					entity->UM.RX_Next_Highest,entity->UM.RX_Next_Reassembly,
					entity->UM.RX_Timer_Trigger);
#endif // LTENR_RLC_LOG
}

void LTENR_RLC_UM_RECEIVE_PDU()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_RLCMSG_UMDHDR hdr = LTENR_RLCMSG_UMHDR_GET(packet);
	if (hdr->si == SI_ALL)
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("SN is not configured in hdr.\n"
						"Removing rlc hdr and Sending SDU to PDCP\n");
#endif
		LTENR_RLCMSG_UMDHDR_REMOVE(packet);
		LTENR_CallPDCPIN();
	}
	else
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("Received PDU has SN\n"
						"\tSI = %s\n"
						"\tSN = %d\n"
						"\tSO = %d\n",
						strSI[hdr->si], hdr->SN, hdr->SO);
#endif
		LTENR_RLC_UM_RECEIVE_PDU_WITH_SN();
	}
}
#pragma endregion
