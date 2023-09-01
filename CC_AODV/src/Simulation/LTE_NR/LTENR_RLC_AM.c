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

#pragma region RLCAM_DEFINE
// a SN falls within the transmitting window if TX_Next_Ack <= SN < TX_Next_Ack + AM_Window_Size
#define LTENR_RLCAM_IsSNFallsInTransmittingWindow(entity,sn) (((entity)->AM.TX_Next_Ack <= (sn)) && ((sn) < (entity)->AM.TX_Next_Ack + AM_Window_Size))

//a SN falls within the receiving window if RX_Next <= SN < RX_Next + AM_Window_Size;
#define LTENR_RLCAM_IsSNFallsInReceivingWindow(entity,sn) (((entity)->AM.RX_Next <= (sn)) && ((sn) < (entity)->AM.RX_Next + AM_Window_Size))
#pragma endregion

#pragma region RLCAM_FunctionPrototype
//Reception buffer
static UINT LTENR_RLCAM_ReceptionBuffer_FindSN(ptrLTENR_RLCENTITY entity, UINT sn, char* op);
static bool LTENR_RLCAM_isAllBytesReceivedForSN(ptrLTENR_RLCENTITY entity, UINT sn);
static void LTENR_RLCAM_UpdateReceptionBuffer(ptrLTENR_RLCENTITY entity);

//Poll retransmit timer
static void LTENR_RLCAM_Start_t_PollRetransmitTimer(ptrLTENR_RLCENTITY entity);
static void LTENR_RLCAM_Stop_t_PollRetransmitTimer(ptrLTENR_RLCENTITY entity);

//Status PDU
static void LTENR_RLCAM_STATUS_ADD(ptrLTENR_RLCENTITY entity, SI si, UINT sn, UINT16 so);
static void LTENR_RLCAM_InitiateToTriggerStatusReport(ptrLTENR_RLCENTITY entity, UINT x);
static void LTENR_RLCAM_TriggerStatusReport(ptrLTENR_RLCENTITY entity);
static void LTENR_RLCAM_ConstructStatusPDU(ptrLTENR_RLCENTITY entity);

//Helper function
static UINT LTENR_RLCAM_UpdateRX_Highest_Status(ptrLTENR_RLCENTITY entity);

//Status
static void LTENR_RLCAM_ClearReceivedInfo(ptrLTENR_RLCENTITY entity, UINT sn);

//State Variable
static UINT LTENR_RLCAM_UpdateRxNext(ptrLTENR_RLCENTITY entity);
#pragma endregion

#pragma region RLCAM_t_Reassembly_timer
void LTENR_RLCAM_Start_t_ReassemblyTimer(ptrLTENR_RLCENTITY entity)
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

void LTENR_RLCAM_Stop_t_ReassemblyTimer(ptrLTENR_RLCENTITY entity)
{
	if (!entity->isReassemblyTimerStarted)
		return;
	fnDeleteEvent(entity->reassemblyTimerId);
	entity->isReassemblyTimerStarted = false;
}

static void LTENR_RLCAM_IncreaseReassembleCount(ptrLTENR_RLCENTITY entity)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = entity->AM.receptionBuffer;
	while (buffer)
	{
		if (buffer->SN <= entity->AM.RX_Highest_Status)
			buffer->reassembleCount++;
		buffer = LIST_NEXT(buffer);
	}
}

void LTENR_RLCAM_Handle_t_reassemblyTimer()
{
	ptrLTENR_RLCENTITY entity = pstruEventDetails->szOtherDetails;
	entity->isReassemblyTimerStarted = false;

	print_ltenr_log("RLCAM- T_Reassembly Timer is triggerd at %0.3lf ms for Device %d-%d, Peer Device %d-%d\n",
					pstruEventDetails->dEventTime / MILLISECOND,
					entity->d, entity->in,
					entity->peerId, entity->peerIf);

	LTENR_RLCAM_UpdateReceptionBuffer(entity);
	entity->AM.RX_Highest_Status = LTENR_RLCAM_UpdateRX_Highest_Status(entity);
	entity->AM.RX_Next = LTENR_RLCAM_UpdateRxNext(entity);
	
	if (entity->AM.RX_Next_Highest > entity->AM.RX_Highest_Status + 1 ||
		(entity->AM.RX_Next_Highest == entity->AM.RX_Highest_Status + 1 &&
		 !LTENR_RLCAM_isAllBytesReceivedForSN(entity, entity->AM.RX_Highest_Status)))
	{
		LTENR_RLCAM_Start_t_ReassemblyTimer(entity);
		entity->AM.RX_Next_Status_Trigger = entity->AM.RX_Next_Highest;
	}

	LTENR_RLCAM_TriggerStatusReport(entity);
	LTENR_RLCAM_IncreaseReassembleCount(entity);
}
#pragma endregion

#pragma region RLCAM_RECEPTIONBUFFER
static UINT LTENR_RLCAM_ReceptionBuffer_FindSN(ptrLTENR_RLCENTITY entity, UINT sn, char* op)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = entity->AM.receptionBuffer;
	while (buffer)
	{
		if (CompareVal(buffer->SN, sn, op))
			return buffer->SN;
		buffer = LIST_NEXT(buffer);
	}
	return 0;
}

static ptrLTENR_RLC_RECEPTIONBUFFER LTENR_RLCAM_FINDRECEPTIONBUFFER(ptrLTENR_RLCENTITY entity,
																	 UINT sn)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buf = entity->AM.receptionBuffer;
	while (buf)
	{
		if (buf->SN == sn)
			return buf;
		buf = LTENR_RLC_RECEPTIONBUFFER_NEXT(buf);
	}
	return NULL;
}

static bool LTENR_RLCAM_IsPDUPresentInReceptionBuffer(ptrLTENR_RLCENTITY entity,
													  UINT sn, UINT16 so)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buf = LTENR_RLCAM_FINDRECEPTIONBUFFER(entity, sn);
	if (!buf)
		return false;
	NetSim_PACKET* p = buf->head;
	while (p)
	{
		ptrLTENR_RLCMSG_AMDHDR hdr = LTENR_RLCMSG_AMHDR_GET(p);
		if (hdr->SO == so)
			return true;
		p = p->pstruNextPacket;
	}
	return false;
}

static ptrLTENR_RLC_RECEPTIONBUFFER LTENR_RLCAM_CREATERECEPTIONBUFFER(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buf = LTENR_RLC_RECEPTIONBUFFER_ALLOC();
	buf->SN = sn;
	LTENR_RLC_RECEPTIONBUFFER_ADD(&entity->AM.receptionBuffer, buf);
	return buf;
}

static void LTENR_RLCAM_DeleteReceptionBuffer(ptrLTENR_RLCENTITY entity,
											   ptrLTENR_RLC_RECEPTIONBUFFER buffer)
{
	LIST_REMOVE(&entity->AM.receptionBuffer, buffer);
}

static void LTENR_RLCAM_AddToReceptionBuffer(ptrLTENR_RLC_RECEPTIONBUFFER buffer,
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

	ptrLTENR_RLCMSG_AMDHDR h = LTENR_RLCMSG_AMHDR_GET(packet);
	if (h->si == SI_LAST || h->si == SI_ALL)
	{
		buffer->isLastSDUReceived = true;
		buffer->lastSDUSO = h->SO;
	}
}

static bool LTENR_RLCAM_isAllByteSegmentReceived(ptrLTENR_RLC_RECEPTIONBUFFER buffer)
{
	NetSim_PACKET* p = buffer->head;
	if (!buffer->isLastSDUReceived)
		return false;
	bool* isRecv;
	isRecv = calloc((size_t)buffer->lastSDUSO + 1, sizeof* isRecv);

	while (p)
	{
		ptrLTENR_RLCMSG_AMDHDR h = LTENR_RLCMSG_AMHDR_GET(p);
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

static bool LTENR_RLCAM_isAllBytesReceivedForSN(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = LTENR_RLCAM_FINDRECEPTIONBUFFER(entity, sn);
	if (!buffer)
		return true;
	return LTENR_RLCAM_isAllByteSegmentReceived(buffer);
}

static NetSim_PACKET* LTENR_RLCAM_ReassembleAllBytes(ptrLTENR_RLC_RECEPTIONBUFFER buffer)
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

static UINT LTENR_RLCAM_UpdateRX_Highest_Status(ptrLTENR_RLCENTITY entity)
{
	UINT ret = 0;
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = entity->AM.receptionBuffer;

	if (!buffer)
		return entity->AM.RX_Next_Highest;

	while (buffer)
	{
		ret = max(ret, buffer->SN);
		buffer = LIST_NEXT(buffer);
	}
	return ret;
}

static UINT LTENR_RLCAM_UpdateRxNext(ptrLTENR_RLCENTITY entity)
{
	UINT ret = (UINT)-1;
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = entity->AM.receptionBuffer;
	if (!buffer)
		return entity->AM.RX_Next_Highest;

	while (buffer)
	{
		ret = min(ret, buffer->SN);
		buffer = LIST_NEXT(buffer);
	}
	return ret;
}

static void LTENR_RLCAM_PLACE_AMDPDU_IN_RECEPTIONBUFFER(ptrLTENR_RLCENTITY entity,
														NetSim_PACKET* packet)
{
	ptrLTENR_RLCMSG_AMDHDR hdr = LTENR_RLCMSG_AMHDR_GET(packet);
	UINT x = hdr->SN;

	ptrLTENR_RLC_RECEPTIONBUFFER buffer = LTENR_RLCAM_FINDRECEPTIONBUFFER(entity, hdr->SN);
	if (!buffer)
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("No recption buffer found for SN %d. Creating new buffer.\n", hdr->SN);
#endif
		buffer = LTENR_RLCAM_CREATERECEPTIONBUFFER(entity, hdr->SN);
	}
	LTENR_RLCAM_AddToReceptionBuffer(buffer, packet);

	LTENR_RLCAM_STATUS_ADD(entity, hdr->si, hdr->SN, hdr->SO);

	if (x >= entity->AM.RX_Next_Highest)
		entity->AM.RX_Next_Highest = x + 1;

	if (LTENR_RLCAM_isAllByteSegmentReceived(buffer))
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("All bytes for SDU with SN %d received.\n"
						"Reassembly all bytes.\n", hdr->SN);
#endif
		NetSim_PACKET* reassemblePacket = LTENR_RLCAM_ReassembleAllBytes(buffer);
		LTENR_RLCMSG_AMDHDR_REMOVE(reassemblePacket);
		LTENR_RLCAM_DeleteReceptionBuffer(entity, buffer);
		buffer = NULL;
#ifdef LTENR_RLC_LOG
		print_ltenr_log("SDU payload size = %d bytes\n"
						"SDU overhead size = %d bytes.\n"
						"SDU total size = %d bytes."
						"Sending SDU to PDCP.\n", (UINT)reassemblePacket->pstruMacData->dPayload,
						(UINT)reassemblePacket->pstruMacData->dOverhead, (UINT)reassemblePacket->pstruMacData->dPacketSize);
#endif

		entity->AM.RX_Highest_Status = LTENR_RLCAM_UpdateRX_Highest_Status(entity);

		entity->AM.RX_Next = LTENR_RLCAM_UpdateRxNext(entity);

		pstruEventDetails->pPacket = reassemblePacket;
		LTENR_CallPDCPIN();	
	}

	if (entity->isReassemblyTimerStarted)
	{
		if (entity->AM.RX_Next_Status_Trigger == entity->AM.RX_Next ||
			(entity->AM.RX_Next_Status_Trigger == entity->AM.RX_Next + 1 &&
			 LTENR_RLCAM_isAllBytesReceivedForSN(entity, entity->AM.RX_Next)) ||
			 (!LTENR_RLCAM_IsSNFallsInReceivingWindow(entity, entity->AM.RX_Next_Status_Trigger) &&
			  entity->AM.RX_Next_Status_Trigger != entity->AM.RX_Next + AM_Window_Size))
		{
			LTENR_RLCAM_Stop_t_ReassemblyTimer(entity);
		}
	}

	if (!entity->isReassemblyTimerStarted)
	{
		if (entity->AM.RX_Next_Highest > entity->AM.RX_Next + 1 ||
			(entity->AM.RX_Next_Highest == entity->AM.RX_Next + 1 &&
			 !LTENR_RLCAM_isAllBytesReceivedForSN(entity, entity->AM.RX_Next)))
		{
			LTENR_RLCAM_Start_t_ReassemblyTimer(entity);
			entity->AM.RX_Next_Status_Trigger = entity->AM.RX_Next_Highest;
		}
	}
}

static void LTENR_RLCAM_UpdateReceptionBuffer(ptrLTENR_RLCENTITY entity)
{
	ptrLTENR_RLC_RECEPTIONBUFFER buffer = entity->AM.receptionBuffer;
	while (buffer)
	{
		if (buffer->reassembleCount > entity->maxRetxThreshold)
		{
			LTENR_RLCAM_ClearReceivedInfo(entity, buffer->SN);
			NetSim_PACKET* packet = buffer->head;
			while (packet)
			{
				NetSim_PACKET* p = packet;
				packet = packet->pstruNextPacket;
				fn_NetSim_Packet_FreePacket(p);
			}
			LIST_FREE(&entity->AM.receptionBuffer, buffer);
			buffer = entity->AM.receptionBuffer;
			continue;
		}
		buffer = LIST_NEXT(buffer);
	}
}
#pragma endregion

#pragma region RLCAM_OUT
void LTENR_RLCAM_HandlePDUFromUpperLayer()
{
	print_ltenr_log("Packet is for AM mode.\n");
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, packet->nReceiverId);
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_AM_TRANSMITTING_ENTITY(d, in,
																	 packet->nReceiverId, rin);

	print_ltenr_log("Adding packet to transmission buffer. SN = %d\n", entity->AM.TX_Next);
	LTENR_LOGICALCHANNEL channel = LTENR_PACKET_GET_LOGICALCHANNEL(packet);
	LTENR_RLCBuffer_AddPacket(&entity->transmissionBuffer, packet, entity->AM.TX_Next, 0, channel);
	entity->AM.TX_Next++;
	entity->bufferStatus[channel] += (UINT)packet->pstruMacData->dPayload;
}

static ptrLTENR_RLCBUFFER LTENR_RLC_AM_FRAGMENT_PACKET(ptrLTENR_RLCBUFFER buffer, UINT size)
{
	UINT s = (UINT)buffer->packet->pstruMacData->dPayload;
	ptrLTENR_RLCBUFFER ret;
	if (s > size)
	{
		ret = LTENR_RLCBUFFER_ALLOC();
		ret->channel = buffer->channel;
		ret->isFragmented = true;
		ret->SN = buffer->SN;
		ret->packet = fn_NetSim_Packet_CopyPacket(buffer->packet);
		ret->packet->pstruMacData->dPayload = size;
		ret->packet->pstruMacData->dPacketSize = ret->packet->pstruMacData->dPayload + ret->packet->pstruMacData->dOverhead;
		buffer->packet->pstruMacData->dPayload -= size;
		buffer->packet->pstruMacData->dPacketSize -= size;
		
		if (buffer->isFragmented)
		{
			ret->si = SI_MIDDLE;
			ret->SO = buffer->SO;

			buffer->si = SI_LAST;
			buffer->SO++;
		}
		else
		{
			ret->si = SI_FIRST;
			ret->SO = 0;

			buffer->si = SI_LAST;
			buffer->SO = 1;
		}
		buffer->isFragmented = true;
		return ret;
	}
	
	if (buffer->isFragmented)
	{
		buffer->si = SI_LAST;
	}
	else
	{
		buffer->si = SI_ALL;
	}
	return buffer;
}

static void LTENR_RLCAM_SendPDUToMac(ptrLTENR_RLCENTITY entity,
									 NetSim_PACKET* amdpdu)
{
	ptrLTENR_RLCMSG_AMDHDR hdr = LTENR_RLCMSG_AMHDR_GET(amdpdu);
	
	LTENR_RLC_SENDPDUTOMAC(entity->d, entity->in, amdpdu);
	entity->AM.TX_HighestSN = max(entity->AM.TX_HighestSN, hdr->SN);

	bool isPollIncluded = false;
	entity->AM.BYTE_WITHOUT_POLL += (UINT)amdpdu->pstruMacData->dPayload;

	if (hdr->si == SI_ALL ||
		hdr->si == SI_LAST)
	{
		entity->AM.PDU_WITHOUT_POLL++;
	}
	else
	{
		return;
	}

	if (entity->AM.PDU_WITHOUT_POLL >= entity->pollPDU ||
		entity->AM.BYTE_WITHOUT_POLL >= entity->pollByte)
		isPollIncluded = true;

	if (!entity->transmissionBuffer && !entity->retransmissionBuffer)
		isPollIncluded = true;

	if (isPollIncluded)
	{
		LTENR_RLCMSG_AMDHDR_SETPOLL(amdpdu);
		entity->AM.BYTE_WITHOUT_POLL = 0;
		entity->AM.PDU_WITHOUT_POLL = 0;

		entity->AM.POLL_SN = entity->AM.TX_HighestSN;

		if (entity->isPollRetransmitTimerStarted)
		{
			LTENR_RLCAM_Stop_t_PollRetransmitTimer(entity);
			LTENR_RLCAM_Start_t_PollRetransmitTimer(entity);
		}
		else
		{
			LTENR_RLCAM_Start_t_PollRetransmitTimer(entity);
		}
	}
}

static UINT LTENR_RLCAM_SendPDUFromRetransmissionBuffer(ptrLTENR_RLCENTITY entity,
														UINT size, LTENR_LOGICALCHANNEL logicalChannel)
{
	ptrLTENR_RLCBUFFER buf = entity->retransmissionBuffer;
	NetSim_PACKET* amdpdu = NULL;
	ptrLTENR_RLCMSG_AMDHDR hdr;
	UINT left = size;
	while (buf)
	{
		amdpdu = buf->packet;

		if (buf->channel != logicalChannel)
		{
			LTENR_RLCBUFFER_NEXT(buf);
			continue;
		}

		if (left < MIN_RLC_PDU_SIZE)
			break;

		UINT sn = buf->SN;
		if (!LTENR_RLCAM_IsSNFallsInTransmittingWindow(entity, sn))
		{
			LTENR_RLCBUFFER_NEXT(buf);
			continue;
		}

		if (amdpdu->pstruMacData->dPayload > left)
		{
			LTENR_RLCBUFFER_NEXT(buf);
			continue;
		}

		hdr = LTENR_RLCMSG_AMHDR_GET(amdpdu);

		left -= (UINT)amdpdu->pstruMacData->dPayload;
		entity->bufferStatus[logicalChannel] -= (UINT)amdpdu->pstruMacData->dPayload;

#ifdef LTENR_RLC_LOG
		print_ltenr_log("Sening packet to MAC sublayer. SI=%s, SN=%d, SO=%d\n",
						strSI[hdr->si], hdr->SN, hdr->SO);
		print_ltenr_log("Adding packet to transmitted buffer. SI=%s, SN=%d, SO=%d\n",
						strSI[hdr->si], hdr->SN, hdr->SO);
#endif

		NetSim_PACKET* sent = fn_NetSim_Packet_CopyPacket(amdpdu);
		LTENR_RLCAM_SendPDUToMac(entity, sent);

		LIST_REMOVE(&entity->retransmissionBuffer, buf);
		LIST_ADD_LAST(&entity->transmittedBuffer, buf);
		buf = entity->retransmissionBuffer;
	}
	return size - left;
}

static UINT LTENR_RLCAM_SendPDUFromTransmissionBuffer(ptrLTENR_RLCENTITY entity,
													  UINT size, LTENR_LOGICALCHANNEL logicalChannel)
{
	ptrLTENR_RLCBUFFER buf = entity->transmissionBuffer;
	UINT left = size;
	while (buf)
	{
		if (buf->channel != logicalChannel)
		{
			LTENR_RLCBUFFER_NEXT(buf);
			continue;
		}

		if (left < MIN_RLC_PDU_SIZE)
			break;

		UINT sn = buf->SN;
		if (!LTENR_RLCAM_IsSNFallsInTransmittingWindow(entity, sn))
		{
			LTENR_RLCBUFFER_NEXT(buf);
			continue;
		}

		ptrLTENR_RLCBUFFER ret = LTENR_RLC_AM_FRAGMENT_PACKET(buf, left);
		if (ret == buf)
		{
			LIST_REMOVE(&entity->transmissionBuffer, buf);
			buf = entity->transmissionBuffer;
		}

		left -= (UINT)ret->packet->pstruMacData->dPayload;
		entity->bufferStatus[logicalChannel] -= (UINT)ret->packet->pstruMacData->dPayload;

#ifdef LTENR_RLC_LOG
		print_ltenr_log("Sening packet to MAC sublayer. SI=%s, SN=%d, SO=%d\n",
						strSI[ret->si], ret->SN, ret->SO);
		print_ltenr_log("Adding packet to transmitted buffer. SI=%s, SN=%d, SO=%d\n",
						strSI[ret->si], ret->SN, ret->SO);
#endif
		LTENR_RLCMSG_AMHDR_ADD(ret->packet, ret->si, ret->SN, ret->SO);

		NetSim_PACKET* sent = fn_NetSim_Packet_CopyPacket(ret->packet);
		LTENR_RLCAM_SendPDUToMac(entity, sent);

		
		LIST_ADD_LAST(&entity->transmittedBuffer, ret);
		
	}
	return size - left;
}

UINT LTENR_RLCAM_SEND_PDU(NETSIM_ID d, NETSIM_ID in,
						  NETSIM_ID r, NETSIM_ID rin,
						  UINT size, LTENR_LOGICALCHANNEL logicalChannel)
{
	UINT sent = 0;
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_AM_TRANSMITTING_ENTITY(d, in, r, rin);
	if (!entity)
		return size - sent;

#ifdef LTENR_RLC_TRANSMISSIONSTATUSNOTIFICATION_LOG
	print_ltenr_log("RLCAM-Sending packet from retransmission buffer.\n");
#endif // LTENR_RLC_LOG
	sent += LTENR_RLCAM_SendPDUFromRetransmissionBuffer(entity, size - sent, logicalChannel);

#ifdef LTENR_RLC_TRANSMISSIONSTATUSNOTIFICATION_LOG
	print_ltenr_log("RLCAM-Sending packet from transmission buffer.\n");
#endif // LTENR_RLC_LOG
	sent += LTENR_RLCAM_SendPDUFromTransmissionBuffer(entity, size - sent, logicalChannel);
	return size - sent;
}
#pragma endregion

#pragma region RLCAM_IN
void LTENR_RLC_AM_RECEIVE_PDU()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_RLCMSG_AMDHDR hdr = LTENR_RLCMSG_AMHDR_GET(packet);
	NETSIM_ID r = packet->nTransmitterId;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, r);
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_AM_RECEIVING_ENTITY(d, in, r, rin);

	if (!entity)
		return;

#ifdef LTENR_RLC_LOG
	print_ltenr_log("\tReceived AMDPDU HDR\n"
					"\t\tSI = %s\n"
					"\t\tSN = %d\n"
					"\t\tSO = %d\n"
					"\t\tPoll bit = %d\n",
					strSI[hdr->si], hdr->SN, hdr->SO, hdr->P); 
#endif
	if (!LTENR_RLCAM_IsSNFallsInReceivingWindow(entity,hdr->SN))
	{
		print_ltenr_log("Received AMDPDU is outside of the receiving window. Discarding PDU\n");
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}

	if (LTENR_RLCAM_IsPDUPresentInReceptionBuffer(entity, hdr->SN, hdr->SO))
	{
		print_ltenr_log("Received AMDPDU is already received. Discarding PDU\n");
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}

	LTENR_RLCAM_PLACE_AMDPDU_IN_RECEPTIONBUFFER(entity, packet);

	if (hdr->P || entity->AM.isStatusReportDelayed)
		LTENR_RLCAM_InitiateToTriggerStatusReport(entity, hdr->SN);
}
#pragma endregion

#pragma region RLCAM_ARQ
#pragma endregion

#pragma region RLCAM_T_POLLRetransmit
static void LTENR_RLCAM_Start_t_PollRetransmitTimer(ptrLTENR_RLCENTITY entity)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime + entity->t_PollRetransmit;
	pevent.nDeviceId = entity->d;
	pevent.nDeviceType = DEVICE_TYPE(entity->d);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = entity->in;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(entity->d, entity->in);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVNT_RLC_TPOLLRETRANSMITTIMER);
	pevent.szOtherDetails = entity;
	entity->pollRetransmitTimerId = fnpAddEvent(&pevent);
	entity->isPollRetransmitTimerStarted = true;
}

static void LTENR_RLCAM_Stop_t_PollRetransmitTimer(ptrLTENR_RLCENTITY entity)
{
	if (!entity->isPollRetransmitTimerStarted)
		return;
	fnDeleteEvent(entity->pollRetransmitTimerId);
	entity->isPollRetransmitTimerStarted = false;
}

void LTENR_RLCAM_Handle_t_PollRetransmitTimer()
{
#ifdef LTENR_RLC_LOG
	print_ltenr_log("RLCAM-Poll retransmit timer triggered at time %lfms.\n",
					pstruEventDetails->dEventTime/MILLISECOND);
#endif // LTENR_RLC_LOG

	ptrLTENR_RLCENTITY entity = pstruEventDetails->szOtherDetails;
	entity->isPollRetransmitTimerStarted = false;

	ptrLTENR_RLCBUFFER buffer = entity->transmittedBuffer;
	while (buffer)
	{
		if (buffer->SN <= entity->AM.POLL_SN)
		{
			ptrLTENR_RLCMSG_AMDHDR hdr = LTENR_RLCMSG_AMHDR_GET(buffer->packet);
			buffer->retxCount++;
			if (buffer->retxCount > entity->maxRetxThreshold)
			{
#ifdef  LTENR_RLC_LOG
				print_ltenr_log("RLCAM- retxCount is reached maxRetxThreshold.Dropping packet.\n");
				print_ltenr_log("Packet Info - SI = %s, SN = %d, SO = %d\n",
								strSI[hdr->si], hdr->SN, hdr->SO);
#endif //  LTENR_RLC_LOG

				fn_NetSim_Packet_FreePacket(buffer->packet);
				LIST_FREE(&entity->transmittedBuffer, buffer);
				buffer = entity->transmittedBuffer;
				continue;
			}
#ifdef  LTENR_RLC_LOG
			print_ltenr_log("RLCAM- Ack is not received. Moving packet from transmitted buffer to retransmission buffer.\n");
			print_ltenr_log("Packet Info - SI = %s, SN = %d, SO = %d\n",
							strSI[hdr->si], hdr->SN, hdr->SO);
#endif //  LTENR_RLC_LOG

			hdr->P = false;
			LIST_REMOVE(&entity->transmittedBuffer, buffer);
			LIST_ADD_LAST(&entity->retransmissionBuffer, buffer);
			entity->bufferStatus[buffer->channel] += (UINT)buffer->packet->pstruMacData->dPayload;
			buffer = entity->transmittedBuffer;
			continue;
		}
		LTENR_RLCBUFFER_NEXT(buffer);
	}
#ifdef LTENR_RLC_LOG
	print_ltenr_log("\n\n");
#endif // LTENR_RLC_LOG

}
#pragma endregion

#pragma region RLCAM_STATUSREPORT
static void LTENR_RLCAM_STATUS_ADD(ptrLTENR_RLCENTITY entity, SI si, UINT sn, UINT16 so)
{
	ptrLTENR_RLCAM_STATUS status = LTENR_RLCAM_STATUS_ALLOC();
	status->si = si;
	status->SN = sn;
	status->SO = so;
	LIST_ADD_LAST((void**)&entity->AM.recvdInfo, status);
}

static ptrLTENR_RLCAM_STATUS LTENR_RLCAM_FindLastStatusInfo(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLCAM_STATUS ret = NULL;
	ptrLTENR_RLCAM_STATUS s = entity->AM.recvdInfo;
	while (s)
	{
		if (s->SN == sn)
		{
			if (s->si == SI_ALL)
				return s;
			if (s->si == SI_LAST)
				return s;
			if (!ret)
				ret = s;
			else if (ret->SO < s->SO)
				ret = s;
		}
		LTENR_RLCAM_STATUS_NEXT(s);
	}
	return ret;
}

static ptrLTENR_RLCAM_STATUS LTENR_RLCAM_FindStatus(ptrLTENR_RLCENTITY entity, UINT sn, UINT16 so)
{
	ptrLTENR_RLCAM_STATUS s = entity->AM.recvdInfo;
	while (s)
	{
		if (s->SN == sn && s->SO == so)
			return s;
		LTENR_RLCAM_STATUS_NEXT(s);
	}
	return NULL;
}

static void LTENR_RLCAM_DeleteFromStatusInfo(ptrLTENR_RLCENTITY entity)
{
	ptrLTENR_RLCAM_STATUS s = entity->AM.recvdInfo;
	while (s)
	{
		if (s->SN < entity->AM.RX_Next)
		{
			LIST_FREE(&entity->AM.recvdInfo, s);
			s = entity->AM.recvdInfo;
			continue;
		}
		s = LIST_NEXT(s);
	}
}

static void LTENR_RLCAM_ClearReceivedInfo(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLCAM_STATUS s = entity->AM.recvdInfo;
	while (s)
	{
		if (s->SN == sn)
		{
			LIST_FREE(&entity->AM.recvdInfo, s);
			s = entity->AM.recvdInfo;
			continue;
		}
		s = LIST_NEXT(s);
	}
}

static void LTENR_RLCAM_TriggerStatusReport(ptrLTENR_RLCENTITY entity)
{
	if (entity->isStatusProhibitTimerStarted)
	{
		entity->AM.isConstructStatusReortDelayed = true;
	}
	else
	{
		LTENR_RLCAM_ConstructStatusPDU(entity);
	}
}

static void LTENR_RLCAM_InitiateToTriggerStatusReport(ptrLTENR_RLCENTITY entity, UINT x)
{
	if (x < entity->AM.RX_Next_Highest ||
		x >= entity->AM.RX_Next + AM_Window_Size)
	{
		entity->AM.isStatusReportDelayed = false;
		LTENR_RLCAM_TriggerStatusReport(entity);
	}
	else
	{
		entity->AM.isStatusReportDelayed = true;
	}

}
#pragma endregion

#pragma region RLCAM_t_StatusProhibit
static void LTENR_RLCAM_Start_t_StatusProhibitTimer(ptrLTENR_RLCENTITY entity)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime + entity->t_StatusProhibit;
	pevent.nDeviceId = entity->d;
	pevent.nDeviceType = DEVICE_TYPE(entity->d);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = entity->in;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(entity->d, entity->in);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVNT_RLC_TSTATUSPROHIBITTIMER);
	pevent.szOtherDetails = entity;
	entity->statusProhibitTimerId = fnpAddEvent(&pevent);
	entity->isStatusProhibitTimerStarted = true;
}

static void LTENR_RLCAM_Stop_t_StatusProhibitTimer(ptrLTENR_RLCENTITY entity)
{
	if (!entity->isStatusProhibitTimerStarted)
		return;
	fnDeleteEvent(entity->statusProhibitTimerId);
	entity->isStatusProhibitTimerStarted = false;
}

void LTENR_RLCAM_Handle_t_StatusProhibitTimer()
{
#ifdef LTENR_RLC_LOG
	print_ltenr_log("RLCAM-Status prohibit timer triggered at time %lfms.\n",
					pstruEventDetails->dEventTime / MILLISECOND);
#endif // LTENR_RLC_LOG

	ptrLTENR_RLCENTITY entity = pstruEventDetails->szOtherDetails;
	entity->isStatusProhibitTimerStarted = false;

	if (entity->AM.isConstructStatusReortDelayed)
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("RLCAM-Construct status report is delayed. Costructing status report.\n");
#endif // LTENR_RLC_LOG

		entity->AM.isConstructStatusReortDelayed = false;
		LTENR_RLCAM_ConstructStatusPDU(entity);
	}

#ifdef LTENR_RLC_LOG
	print_ltenr_log("\n\n");
#endif // LTENR_RLC_LOG

}
#pragma endregion

#pragma region RLCAM_STATUSPDU_SEND
static UINT LTENR_RLCAM_StatusPdu_NewNack(ptrLTENR_RLCMSG_STATUSPDU pdu, SI si, UINT sn, UINT16 so)
{
	LTENR_RLCMSG_STATUSPDU_SETE1(pdu);
	ptrLTENR_RLCMSG_STATUSPDU_NACK nack;
	nack = calloc(1, sizeof * nack);
	if (si == SI_ALL)
	{
		nack->NAck_SN = sn;
		LTENR_RLCMSG_STATUSPDU_AddNack(pdu, nack);
		return LTENR_RLCMSG_STATUSPDU_NACKLEN_0;
	}

	nack->NAck_SN = sn;
	nack->soStart = so;
	nack->soEnd = so;
	nack->E2 = true;
	LTENR_RLCMSG_STATUSPDU_AddNack(pdu, nack);
	return LTENR_RLCMSG_STATUSPDU_NACKLEN_1;
}

static bool LTENR_RLCAM_StatusPdu_UpdateNackTail(ptrLTENR_RLCMSG_STATUSPDU pdu, SI si, UINT sn, UINT16 so, UINT* size)
{
	ptrLTENR_RLCMSG_STATUSPDU_NACK nack = pdu->nackTail;
	if (!nack)
		return false;

	if (si == SI_ALL)
	{
		if (nack->E3)
		{
			if (nack->NAck_SN + nack->NAckRange == sn)
			{
				nack->NAckRange++;
				return true;
			}
			else
				return false;
		}

		if (!nack->E2)
		{
			if (nack->NAck_SN + 1 == sn)
			{
				nack->E3 = true;
				nack->NAckRange = 2;
				*size = LTENR_RLCMSG_STATUSPDU_NACKLEN_2 - LTENR_RLCMSG_STATUSPDU_NACKLEN_0;
				return true;
			}
			else
				return false;
		}
	}
	else
	{
		if (nack->NAck_SN == sn && nack->E2)
		{
			if (nack->soEnd + 1 == so)
			{
				nack->soEnd = so;
				return true;
			}
			else
				return false;
		}
		else
		{
			return false;
		}
	}
	return false;
}

static UINT LTENR_RLCAM_STATUSPDU_AddNack(ptrLTENR_RLCMSG_STATUSPDU pdu, SI si, UINT sn, UINT16 so)
{
	UINT size = 0;
	bool isUpdated = false;
	if (pdu->nackTail)
	{
		isUpdated = LTENR_RLCAM_StatusPdu_UpdateNackTail(pdu, si, sn, so, &size);
		if (!isUpdated)
			size = LTENR_RLCAM_StatusPdu_NewNack(pdu, si, sn, so);
	}
	else
	{
		size = LTENR_RLCAM_StatusPdu_NewNack(pdu, si, sn, so);
	}
	return size;
}

static UINT LTENR_RLCAM_UpdateStatus(ptrLTENR_RLCENTITY entity,ptrLTENR_RLCMSG_STATUSPDU pdu)
{
	UINT sn;
	UINT size = 0;
	for (sn = entity->AM.RX_Next; sn < entity->AM.RX_Next_Highest; sn++)
	{
		ptrLTENR_RLCAM_STATUS s = LTENR_RLCAM_FindLastStatusInfo(entity, sn);
		if (!s)
		{
			size += LTENR_RLCAM_STATUSPDU_AddNack(pdu, SI_ALL, sn, 0);
			continue;
		}

		if (s->si == SI_ALL)
			continue;
		
		UINT16 so;
		UINT16 maxSO = s->SO;
		for (so = 0; so <= maxSO; so++)
		{
			ptrLTENR_RLCAM_STATUS s1 = LTENR_RLCAM_FindStatus(entity, sn, so);
			if (!s1)
			{
				size += LTENR_RLCAM_STATUSPDU_AddNack(pdu, SI_MIDDLE, sn, so);
			}
		}
	}

	LTENR_RLCAM_DeleteFromStatusInfo(entity);
	return size;
}

static void LTENR_RLCAM_ConstructStatusPDU(ptrLTENR_RLCENTITY entity)
{
	ptrLTENR_RLCMSG_STATUSPDU pdu = LTENR_RLCMSG_STATUSPDU_NEW();
	UINT size = LTENR_RLCAM_UpdateStatus(entity, pdu);

	pdu->Ack_SN = entity->AM.RX_Next_Highest;
	NetSim_PACKET* p = LTENR_RLCMSG_STATUSPDU_AddToPacket(entity, pdu, size);
	
#ifdef LTENR_RLC_LOG
	print_ltenr_log("Constructing Status PDU. NACK size = %d, Count = %d\n",
		size, pdu->NACKCount);
	print_ltenr_log("Passing packet to TM mode for transmission\n");
#endif

	NetSim_PACKET* org = pstruEventDetails->pPacket;
	pstruEventDetails->pPacket = p;
	LTENR_RLC_TM_HandlePDUFromUpperLayer();
	pstruEventDetails->pPacket = org;
}
#pragma endregion

#pragma region RLCAM_STATUSPDU_RECEIVE

static bool LTENR_RLCAM_MovePacketFromTransmittedBuffer(ptrLTENR_RLCENTITY entity, ptrLTENR_RLCBUFFER buffer)
{
	ptrLTENR_RLCMSG_AMDHDR hdr = LTENR_RLCMSG_AMHDR_GET(buffer->packet);
#ifdef LTENR_RLC_LOG
	print_ltenr_log("RLCAM--Negative ack arrive for RLC PDU.Retransmission count = %d\n",
		buffer->retxCount);
	print_ltenr_log("\tSI = %s\n"
		"\tSN = %d\n"
		"\tSO = %d\n",
		strSI[hdr->si], hdr->SN, hdr->SO);
#endif
	if (buffer->retxCount == entity->maxRetxThreshold)
	{
#ifdef LTENR_RLC_LOG
		print_ltenr_log("\tMax retransmission is reached for pdu.Dropping PDU.\n");
#endif
		fn_NetSim_Packet_FreePacket(buffer->packet);
		return false;
	}
	else
	{
		hdr->P = false;
		buffer->retxCount++;
		LTENR_RLCBUFFER_ADD(&entity->retransmissionBuffer, buffer);
		entity->bufferStatus[buffer->channel] += (UINT)buffer->packet->pstruMacData->dPayload;
		return true;
	}
}

static void LTENR_RLCAM_UpdateBufferBasedOnNACK0(ptrLTENR_RLCENTITY entity, ptrLTENR_RLCMSG_STATUSPDU_NACK nack)
{
	UINT sn = nack->NAck_SN;
	ptrLTENR_RLCBUFFER buffer = LTENR_RLCBUFFER_FindAndRemoveAllBySN(&entity->transmittedBuffer, sn);
	while (buffer)
	{
		if (LTENR_RLCAM_MovePacketFromTransmittedBuffer(entity, buffer))
			LIST_REMOVE(&buffer, buffer);
		else
			LIST_FREE(&buffer, buffer);
	}
}

static void LTENR_RLCAM_UpdateBufferBasedOnNACK1(ptrLTENR_RLCENTITY entity, ptrLTENR_RLCMSG_STATUSPDU_NACK nack)
{
	UINT sn = nack->NAck_SN;
	UINT16 so;
	ptrLTENR_RLCBUFFER buffer;
	for (so = nack->soStart; so <= nack->soEnd; so++)
	{
		buffer = LTENR_RLCBUFFER_FindAndRemoveBySNSO(&entity->transmittedBuffer, sn, so);
		if (!buffer)
			continue;

		if (!LTENR_RLCAM_MovePacketFromTransmittedBuffer(entity, buffer))
			LIST_FREE(&buffer, buffer);
	}
}

static void LTENR_RLCAM_UpdateBufferBasedOnNACK2(ptrLTENR_RLCENTITY entity, ptrLTENR_RLCMSG_STATUSPDU_NACK nack)
{
	UINT sn;
	ptrLTENR_RLCBUFFER buffer;
	for (sn = nack->NAck_SN; sn <= nack->NAck_SN + nack->NAckRange; sn++)
	{
		buffer = LTENR_RLCBUFFER_FindAndRemoveAllBySN(&entity->transmittedBuffer, sn);
		while (buffer)
		{
			if (LTENR_RLCAM_MovePacketFromTransmittedBuffer(entity, buffer))
				LIST_REMOVE(&buffer, buffer);
			else
				LIST_FREE(&buffer, buffer);
		}
	}
}

static void LTENR_RLCAM_ClearTransmittedBuffer(ptrLTENR_RLCENTITY entity, UINT sn)
{
	ptrLTENR_RLCBUFFER buf = entity->transmittedBuffer;
	while (buf)
	{
		if (buf->SN < sn)
		{
			fn_NetSim_Packet_FreePacket(buf->packet);
			LIST_FREE(&entity->transmittedBuffer, buf);
			buf = entity->transmittedBuffer;
			continue;
		}
		LTENR_RLCBUFFER_NEXT(buf);
	}
}

void LTENR_RLCAM_ReceiveStatusPDU()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;

	NETSIM_ID r = packet->nTransmitterId;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, r);
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_AM_TRANSMITTING_ENTITY(d, in, r, rin);
	if (!entity)
		return;
	ptrLTENR_RLCMSG_STATUSPDU statusPdu = LTENR_RLCMSG_STATUSPDU_GET(packet);

	if (LTENR_RLCAM_STATUSPDU_FindSN(statusPdu, entity->AM.POLL_SN))
	{
		if (entity->isPollRetransmitTimerStarted)
		{
			LTENR_RLCAM_Stop_t_PollRetransmitTimer(entity);
			LTENR_RLCAM_Start_t_PollRetransmitTimer(entity);
		}
	}

	ptrLTENR_RLCMSG_STATUSPDU_NACK nack = statusPdu->nackHead;
	while (nack)
	{
		if (nack->E3)
		{
#ifdef LTENR_RLC_LOG
			print_ltenr_log("\tRLCAM- Nack Type 3\n");
			print_ltenr_log("\t\tSN = %d\n"
							"\t\tNack range = %d\n\n",
							nack->NAck_SN, nack->NAckRange);
#endif
			LTENR_RLCAM_UpdateBufferBasedOnNACK2(entity, nack);
		}
		else if (nack->E2)
		{
#ifdef LTENR_RLC_LOG
			print_ltenr_log("\tRLCAM- Nack Type 2\n");
			print_ltenr_log("\t\tSN = %d\n"
							"\t\tSO Start = %d\n"
							"\t\tSO end = %d\n\n",
							nack->NAck_SN, nack->soStart, nack->soEnd);
#endif
			LTENR_RLCAM_UpdateBufferBasedOnNACK1(entity, nack);
		}
		else
		{
#ifdef LTENR_RLC_LOG
			print_ltenr_log("\tRLCAM- Nack Type 1\n");
			print_ltenr_log("\t\tSN = %d\n\n",
							nack->NAck_SN);
#endif
			LTENR_RLCAM_UpdateBufferBasedOnNACK0(entity, nack);
		}

		nack = nack->next;
	}

	LTENR_RLCAM_ClearTransmittedBuffer(entity, statusPdu->Ack_SN);
	
	if (entity->transmissionBuffer)
		entity->AM.TX_Next_Ack = entity->transmissionBuffer->SN;
	else
		entity->AM.TX_Next_Ack = entity->AM.TX_Next;

	if (entity->AM.POLL_SN < statusPdu->Ack_SN)
	{
		LTENR_RLCAM_Stop_t_PollRetransmitTimer(entity);
	}
	
	fn_NetSim_Packet_FreePacket(packet);
	pstruEventDetails->pPacket = NULL;
}
#pragma endregion
