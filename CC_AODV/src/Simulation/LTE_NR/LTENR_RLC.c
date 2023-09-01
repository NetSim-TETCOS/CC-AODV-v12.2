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

#pragma region RLC_GLOBAL_VAIABLE
#pragma endregion

#pragma region RLC_FUNCTIONPROTOTYPE
static void LTENR_RLC_ESTABLISHENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf,
	LTENR_RLCMODE mode,
	bool isTransmitting,
	bool isReceiving);
void LTENR_RLC_ESTABLISHENTITYALL(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf);
static void LTENR_RLC_ESTABLISHBROADCASTENTITY(NETSIM_ID d, NETSIM_ID in);
void LTENR_RLC_Handle_t_reassemblyTimer();
#pragma endregion

#pragma region RLC_INIT
void fn_NetSim_LTENR_RLC_Init()
{
	LTENR_RLCMSG_INIT();

	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVNT_RLC_TREASSEMBLYTIMER,
		"LTENR_RLC_t_reassemblyTimer",
		LTENR_RLC_Handle_t_reassemblyTimer);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVNT_RLC_TPOLLRETRANSMITTIMER,
		"LTENR_RLCAM_t_PollRetransmitTimer",
		LTENR_RLCAM_Handle_t_PollRetransmitTimer);

	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVNT_RLC_TSTATUSPROHIBITTIMER,
		"LTENR_RLCAM_t_StatusProhibitTimer",
		LTENR_RLCAM_Handle_t_StatusProhibitTimer);
	fn_NetSim_LTENR_RegisterCallBackForAssociation(LTENR_RLC_ASSOCIATION);
}

void fn_NetSim_LTENR_UERLC_Init(NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(ueId, ueIf);

	ptrLTENR_RLCVAR rlc = calloc(1, sizeof * rlc);
	LTENR_UERLC_SET(ueId, ueIf, rlc);

	rlc->t_reassembly = str_to_time(pd->t_reassembly);

	ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(ueId, ueIf, 0, 0);
	while (info)
	{
		//LTENR_RLC_ESTABLISHENTITYALL(ueId, ueIf, info->d, info->in);
		info = info->next;
	}

	LTENR_RLC_ESTABLISHBROADCASTENTITY(ueId, ueIf);
}

void fn_NetSim_LTENR_GNBRLC_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	ptrLTENR_RLCVAR rlc = calloc(1, sizeof * rlc);
	LTENR_GNBRLC_SET(gnbId, gnbIf, rlc);

	rlc->t_reassembly = str_to_time(pd->t_reassembly);

	ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(gnbId, gnbIf, 0, 0);
	while (info)
	{
		//LTENR_RLC_ESTABLISHENTITYALL(gnbId, gnbIf, info->d, info->in);
		info = info->next;
	}

	LTENR_RLC_ESTABLISHBROADCASTENTITY(gnbId, gnbIf);
}
#pragma endregion

#pragma region RLC_ENTITY
ptrLTENR_RLCENTITY LTENR_RLC_FINDENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf,
	LTENR_RLCMODE mode,
	bool isTransmitting,
	bool isReceiving)
{
	ptrLTENR_RLCVAR rlc = LTENR_RLC_GET(d, in);
	ptrLTENR_RLCENTITY entity = rlc->rlcEntities;
	while (entity)
	{
		if (entity->d == d && entity->in == in &&
			entity->peerId == peerId && entity->peerIf == peerIf &&
			entity->rlcMode == mode &&
			entity->isTransmittingRLCEntity == isTransmitting &&
			entity->isReceivingRLCEntity == isReceiving)
			return entity;
		entity = LTENR_RLCENTITY_NEXT(entity);
	}
	return NULL;
}


static void LTENR_RLC_ESTABLISHENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf,
	LTENR_RLCMODE mode,
	bool isTransmitting,
	bool isReceiving)
{
	ptrLTENR_RLCENTITY entity = LTENR_RLC_FINDENTITY(d, in,
		peerId, peerIf,
		mode, isTransmitting, isReceiving);

	if (entity)
		return;

	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(d, in);
	ptrLTENR_RLCVAR rlc = LTENR_RLC_GET(d, in);
	entity = LTENR_RLCENTITY_ALLOC();

	LTENR_RLCENTITY_ADD(rlc, entity);

	entity->t_Reassembly = rlc->t_reassembly;

	entity->d = d;
	entity->in = in;
	entity->peerId = peerId;
	entity->peerIf = peerIf;

	entity->rlcMode = mode;
	entity->isReceivingRLCEntity = isReceiving;
	entity->isTransmittingRLCEntity = isTransmitting;

	//Config parameter
	entity->t_PollRetransmit = str_to_time(pd->t_pollRetransmit);
	entity->t_StatusProhibit = str_to_time(pd->t_statusProhibit);
	entity->t_Reassembly = str_to_time(pd->t_reassembly);
	entity->pollByte = str_to_bytes(pd->pollBytes);
	entity->pollPDU = atoi(pd->pollPdu + 1);
	entity->maxRetxThreshold = atoi(pd->maxRetxThreshold + 1);
}

static void LTENR_RLC_UNESTABLISHENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf,
	LTENR_RLCMODE mode,
	bool isTransmitting,
	bool isReceiving)
{
	ptrLTENR_RLCVAR rlc = LTENR_RLC_GET(d, in);
	ptrLTENR_RLCENTITY entity = LTENR_RLC_FINDENTITY(d, in,
		peerId, peerIf,
		mode, isTransmitting, isReceiving);

	LTENR_RLCENTITY_REMOVE(rlc, entity);
}

void LTENR_RLC_ESTABLISHENTITYALL(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf)
{
	LTENR_RLC_ESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_TM, true, false);
	LTENR_RLC_ESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_TM, false, true);
	LTENR_RLC_ESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_UM, true, false);
	LTENR_RLC_ESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_UM, false, true);
	LTENR_RLC_ESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_AM, true, true);
}

void LTENR_RLC_UNESTABLISHENTITYALL(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf)
{
	LTENR_RLC_UNESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_TM, true, false);
	LTENR_RLC_UNESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_TM, false, true);
	LTENR_RLC_UNESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_UM, true, false);
	LTENR_RLC_UNESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_UM, false, true);
	LTENR_RLC_UNESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_AM, true, true);
}

void LTENR_RLC_TMESTABLISHENTITYALL(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID peerId, NETSIM_ID peerIf) {
	LTENR_RLC_ESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_TM, true, false);
	LTENR_RLC_ESTABLISHENTITY(d, in, peerId, peerIf, LTENR_RLCMODE_TM, false, true);
}

static void LTENR_RLC_ESTABLISHBROADCASTENTITY(NETSIM_ID d, NETSIM_ID in)
{
	LTENR_RLC_ESTABLISHENTITY(d, in, 0, 0, LTENR_RLCMODE_TM, true, false);
	LTENR_RLC_ESTABLISHENTITY(d, in, 0, 0, LTENR_RLCMODE_TM, false, true);
	LTENR_RLC_ESTABLISHENTITY(d, in, 0, 0, LTENR_RLCMODE_UM, true, false);
	LTENR_RLC_ESTABLISHENTITY(d, in, 0, 0, LTENR_RLCMODE_UM, false, true);
	LTENR_RLC_ESTABLISHENTITY(d, in, 0, 0, LTENR_RLCMODE_AM, true, true);
}
#pragma endregion

#pragma region RLC_ASSOCIATION
void LTENR_RLC_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
	NETSIM_ID ueId, NETSIM_ID ueIf,
	bool isAssociated)
{
	ptrLTENR_RLCVAR grlc = LTENR_GNBRLC_GET(gnbId, gnbIf);
	if (!grlc)
		return; // gNB RLC is still not initiallized.

	ptrLTENR_RLCVAR urlc = LTENR_UERLC_GET(ueId, ueIf);
	if (!urlc)
		return; // UE RLC is still not initiallized.

	if (isAssociated)
	{
		LTENR_RLC_ESTABLISHENTITYALL(gnbId, gnbIf, ueId, ueIf);
		LTENR_RLC_ESTABLISHENTITYALL(ueId, ueIf, gnbId, gnbIf);
	}
	else
	{
		LTENR_RLC_UNESTABLISHENTITYALL(gnbId, gnbIf, ueId, ueIf);
		LTENR_RLC_UNESTABLISHENTITYALL(ueId, ueIf, gnbId, gnbIf);
	}
}
#pragma endregion

#pragma region RLC_TM
void LTENR_RLC_TM_HandlePDUFromUpperLayer()
{
	print_ltenr_log("Packet is TM mode.\n");
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_RLCENTITY entity;
	NETSIM_ID rin = 0;
	if (packet->nReceiverId)
	{
		rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, packet->nReceiverId);
		entity = LTENR_RLC_FINDENTITY(d, in,
			packet->nReceiverId, rin,
			LTENR_RLCMODE_TM,
			true, false);
	}
	else
	{
		entity = LTENR_RLC_FINDENTITY(d, in,
			0, 0,
			LTENR_RLCMODE_TM,
			true, false);
	}

	if (!entity)
	{
		fnNetSimError("LTENR-RLCTM, No entity is found between device %d-%d and %d-%d\n"
			"Dropping packet.\n",
			d, in, packet->nReceiverId, rin);
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}

	print_ltenr_log("Adding packet to transmission buffer.\n");
	LTENR_LOGICALCHANNEL channel = LTENR_PACKET_GET_LOGICALCHANNEL(packet);
	LTENR_RLCBuffer_AddPacket(&entity->transmissionBuffer, packet, 0, 0, channel);
	entity->bufferStatus[channel] += (UINT)packet->pstruMacData->dPayload;
}

static UINT LTENR_RLC_TM_SEND_PDU(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	UINT size, LTENR_LOGICALCHANNEL logicalChannel)
{
	ptrLTENR_RLCENTITY entity = LTENR_RLC_GET_TM_TRANSMITTING_ENTITY(d, in, r, rin);
	NetSim_PACKET* tmdpdu = NULL;

	UINT left = size;
	ptrLTENR_RLCBUFFER buf = entity->transmissionBuffer;
	while (buf)
	{
		tmdpdu = buf->packet;

		LTENR_LOGICALCHANNEL ch = LTENR_PACKET_GET_LOGICALCHANNEL(tmdpdu);
		if (ch != logicalChannel)
		{
			LTENR_RLCBUFFER_NEXT(buf);
			continue;
		}

		if (LTENR_IS_UPLANE_MSG(tmdpdu) &&
			tmdpdu->pstruMacData->dPayload > left)
		{
			LTENR_RLCBuffer_MarkForRemoval(buf);
			fn_NetSim_Packet_FreePacket(tmdpdu);
			LTENR_RLCBUFFER_NEXT(buf);
			continue;
		}
		LTENR_RLCBuffer_MarkForRemoval(buf);
		LTENR_RLCBUFFER_NEXT(buf);
		left -= (UINT)tmdpdu->pstruMacData->dPayload;
		LTENR_RLCMSG_TMHDR_ADD(tmdpdu);
		LTENR_RLC_SENDPDUTOMAC(d, in, tmdpdu);
	}
	LTENR_RLCBuffer_UpdateBuffer(&entity->transmissionBuffer);
	return size - left;
}

static void LTENR_RLC_TM_ReceivePDU()
{
	NetSim_PACKET* p = pstruEventDetails->pPacket;
	switch (p->nControlDataType)
	{
	case LTENR_MSG_RLC_STATUSPDU:
		LTENR_RLCAM_ReceiveStatusPDU();
		break;
	case LTENR_MSG_RRC_MIB:
	case LTENR_MSG_RRC_SIB1:
	case LTENR_MSG_RRC_SI:
	case LTENR_MSG_RRC_SETUP_REQUEST:
		LTENR_CallRRCIn();
		break;
	default:
		LTENR_CallPDCPIN();
		break;
	}
}
#pragma endregion

#pragma region RLC_PACKETPROCESSING

static bool LTENR_RLC_VALIDATE_PDU(NetSim_PACKET* packet, char* fun)
{
	if (!packet)
	{
		fnNetSimError("LTENR-RLC %s function is called without packet\n", fun);
		return false;
	}

	ptrLTENR_MSG msg = packet->pstruMacData->Packet_MACProtocol;
	if (!msg)
	{
		fnNetSimError("LTENR-RLC %s function is called without LTENR-MSG\n", fun);
		return false;
	}

	if (msg->rlcMode == LTENR_RLCMODE_TM)
	{

		if (!LTENR_RLC_IS_LOGICALCHANNEL_FOR_TM(msg->logicalChannel))
		{
			fnNetSimError("LTENR-RLC TM mode is only supporetd for BCCH, CCCH or PCCH logical channel\n");
			return false;
		}
	}

	if (msg->rlcMode == LTENR_RLCMODE_UM)
	{

		if (!LTENR_RLC_IS_LOGICALCHANNEL_FOR_UM(msg->logicalChannel))
		{
			fnNetSimError("LTENR-RLC UM mode is only supporetd for DTCH logical channel\n");
			return false;
		}
	}

	if (msg->rlcMode == LTENR_RLCMODE_AM)
	{

		if (!LTENR_RLC_IS_LOGICALCHANNEL_FOR_AM(msg->logicalChannel))
		{
			fnNetSimError("LTENR-RLC AM mode is only supporetd for DTCH or DCCH logical channel\n");
			return false;
		}
	}

	return true;
}

void fn_NetSim_LTENR_RLC_HandlePDUFromUpperLayer()
{
	LTENR_RLC_PRINT_PACKET_LOG_OUT();
	NetSim_PACKET* packet = pstruEventDetails->pPacket;

	if (!LTENR_RLC_VALIDATE_PDU(packet, __FUNCTION__))
		return;

	ptrLTENR_MSG msg = packet->pstruMacData->Packet_MACProtocol;
	switch (msg->rlcMode)
	{
	case LTENR_RLCMODE_TM:
		LTENR_RLC_TM_HandlePDUFromUpperLayer();
		break;
	case LTENR_RLCMODE_UM:
		LTENR_RLC_UM_HandlePDUFromUpperLayer();
		break;
	case LTENR_RLCMODE_AM:
		LTENR_RLCAM_HandlePDUFromUpperLayer();
		break;
	default:
		fnNetSimError("LTENR-RLC Unknown RLC mode %d\n", msg->rlcMode);
		break;
	}
	print_ltenr_log("\n");
}

void fn_NetSim_LTENR_RLC_TransmissionStatusNotification(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	UINT size, LTENR_LOGICALCHANNEL logicalChannel)
{
#ifdef LTENR_RLC_TRANSMISSIONSTATUSNOTIFICATION_LOG
	print_ltenr_log("LTENR-RLC, Transmission status notification\n"
		"\tTime = %lf us\n"
		"\tDevice Id = %d\n"
		"\tInterface Id = %d\n"
		"\tRemote device id = %d\n"
		"\tRemote device interface = %d\n"
		"\tLogical channel = %s\n",
		pstruEventDetails->dEventTime,
		d, in, r, rin,
		strLTENR_LOGICALCHANNEL[logicalChannel]);
#endif

	UINT tsize, usize, asize;
	tsize = usize = asize = 0;
	if (LTENR_RLC_IS_LOGICALCHANNEL_FOR_TM(logicalChannel))
	{
		tsize = LTENR_RLC_TM_SEND_PDU(d, in, r, rin, size, logicalChannel);
		size -= tsize;
	}

	if (LTENR_RLC_IS_LOGICALCHANNEL_FOR_UM(logicalChannel))
	{
		usize = LTENR_RLC_UM_SEND_PDU(d, in, r, rin, size, logicalChannel);
		size -= usize;
	}

	if (LTENR_RLC_IS_LOGICALCHANNEL_FOR_AM(logicalChannel))
	{
		asize = LTENR_RLCAM_SEND_PDU(d, in, r, rin, size, logicalChannel);
		size -= asize;
	}
#ifdef LTENR_RLC_TRANSMISSIONSTATUSNOTIFICATION_LOG
	print_ltenr_log("\tTM bytes sent = %d\n"
		"\tUM bytes sent = %d\n"
		"\tAM bytes sent = %d\n"
		"\tTotal bytes sent = %d\n",
		tsize, usize, asize,
		tsize + usize + asize);
	print_ltenr_log("\n");
#endif
}

UINT fn_NetSim_LTENR_RLC_BufferStatusNotificaton(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	LTENR_LOGICALCHANNEL logicalChannel)
{
	ptrLTENR_RLCENTITY entity;
	UINT tsize, usize, asize, size;
	tsize = usize = asize = size = 0;
	if (LTENR_RLC_IS_LOGICALCHANNEL_FOR_TM(logicalChannel))
	{
		entity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_TM, true, false);
		if(entity)
			tsize = entity->bufferStatus[logicalChannel];
	}
	if (LTENR_RLC_IS_LOGICALCHANNEL_FOR_UM(logicalChannel))
	{
		entity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_UM, true, false);
		if(entity)
			usize = entity->bufferStatus[logicalChannel];
	}
	if (LTENR_RLC_IS_LOGICALCHANNEL_FOR_AM(logicalChannel))
	{
		entity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_AM, true, true);
		if(entity)
			asize = entity->bufferStatus[logicalChannel];
	}

	size = tsize + usize + asize;
#ifdef LTENR_RLC_BUFFERSTATUSREPORTING_LOG
	print_ltenr_log("LTENR-RLC, Buffer status reporting\n"
		"\tTime = %lf us\n"
		"\tDevice Id = %d\n"
		"\tInterface Id = %d\n"
		"\tRemote device id = %d\n"
		"\tRemote device interface = %d\n"
		"\tLogical channel = %s\n"
		"\tTM buffer size = %d bytes\n"
		"\tUM buffer size = %d bytes\n"
		"\tAM buffer size = %d bytes\n"
		"\tTotal buffer size = %d bytes\n",
		pstruEventDetails->dEventTime,
		d, in, r, rin,
		strLTENR_LOGICALCHANNEL[logicalChannel],
		tsize, usize, asize, size);
	print_ltenr_log("\n");
#endif
	return size;
}

void fn_NetSim_LTENR_RLC_HandleMACIN()
{
	LTENR_RLC_PRINT_PACKET_LOG_IN();
	LTENR_RLCMODE mode = LTENR_PACKET_GET_RLCMODE(pstruEventDetails->pPacket);
	switch (mode)
	{
	case LTENR_RLCMODE_TM:
		LTENR_RLC_TM_ReceivePDU();
		break;
	case LTENR_RLCMODE_UM:
		LTENR_RLC_UM_RECEIVE_PDU();
		break;
	case LTENR_RLCMODE_AM:
		LTENR_RLC_AM_RECEIVE_PDU();
		break;
	default:
		fnNetSimError("Packet arrives to RLC in has no valid RLC mode.\n"
			"Dropping packet.\n");
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
		pstruEventDetails->pPacket = NULL;
		break;
	}
}
#pragma endregion

#pragma region RLC_t_ReassemblyTime
void LTENR_RLC_Handle_t_reassemblyTimer()
{
	ptrLTENR_RLCENTITY entity = pstruEventDetails->szOtherDetails;
	LTENR_RLCMODE mode = entity->rlcMode;

	switch (mode)
	{
	case LTENR_RLCMODE_UM:
		LTENR_RLC_UM_Handle_t_reassemblyTimer();
		break;
	case LTENR_RLCMODE_AM:
		LTENR_RLCAM_Handle_t_reassemblyTimer();
		break;
	default:
		fnNetSimError("t_reassembly timer is triggered without valid rlc mode.\n");
		break;
	}
}
#pragma endregion
