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
#include "LTENR_PDCP.h"

#ifndef LTENR_PDCP_LOG
#define print_ltenr_log
#define print_ltenr_packet_log
#endif

#pragma region PDCP_GLOBALVARIABLE
static UINT PDCPHDRID = 0;
#pragma endregion

#pragma region PDCP_FUNCTIONPROTOTYPE
void LTENR_PDCP_HANDLE_DISCARD_TIMER();
void LTENR_PDCP_HANDLE_TREORDERINGTIMER();
void LTENR_PDCP_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
	NETSIM_ID ueId, NETSIM_ID ueIf,
	bool isAssociated);

void LTENR_PDCP_HDR_Trace(ptrLTENR_PDCP_HDR hdr, char* s);
void LTENR_PDCP_HDR_FREE(ptrLTENR_PDCP_HDR hdr);
ptrLTENR_PDCP_HDR LTENR_PDCP_HDR_COPY(ptrLTENR_PDCP_HDR hdr);
#pragma endregion

#pragma region PDCP

void fn_NetSim_LTENR_PDCP_Init()
{
	PDCPHDRID = LTENR_HDR_REGISTER("PDCP Hdr",
		NULL,
		LTENR_PDCP_HDR_COPY,
		LTENR_PDCP_HDR_FREE,
		LTENR_PDCP_HDR_Trace);

	//Subevent registration
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PDCP_DISCARDTIMER,
		"PDCP_DISCARDTIMER",
		LTENR_PDCP_HANDLE_DISCARD_TIMER);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PDCP_TREORDERINGTIMER,
		"PDCP_TREORDERING",
		LTENR_PDCP_HANDLE_TREORDERINGTIMER);

	fn_NetSim_LTENR_RegisterCallBackForAssociation(LTENR_PDCP_ASSOCIATION);
}

void fn_NetSim_LTENR_GNBPDCP_Init(NETSIM_ID gnbId,
	NETSIM_ID gnbIf)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	ptrLTENR_PDCPVAR pdcp = calloc(1, sizeof * pdcp);
	LTENR_GNBPDCP_SET(gnbId, gnbIf, pdcp);

	pdcp->isoutOfOrderDelivery = pd->isoutOfOrderDelivery;
	pdcp->isHdrCompression = pd->isHdrCompression;
	pdcp->discardDelayTimer = pd->discardTimer;
	pdcp->t_Reordering = pd->t_reorderingTimer;


	ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(gnbId, gnbIf, 0, 0);
	while (info)
	{
		//LTENR_PDCP_ASSOCIATION(gnbId, gnbIf,
		//					   info->d, info->in,
		//					   true);
		info = info->next;
	}
}

void fn_NetSim_LTENR_UEPDCP_Init(NETSIM_ID ueId,
	NETSIM_ID ueIf)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(ueId, ueIf);
	ptrLTENR_PDCPVAR pdcp = calloc(1, sizeof * pdcp);
	LTENR_UEPDCP_SET(ueId, ueIf, pdcp);

	pdcp->isoutOfOrderDelivery = pd->isoutOfOrderDelivery;
	pdcp->isHdrCompression = pd->isHdrCompression;
	pdcp->discardDelayTimer = pd->discardTimer;
	pdcp->t_Reordering = pd->t_reorderingTimer;

	ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(ueId, ueIf, 0, 0);
	while (info)
	{
		//LTENR_PDCP_ASSOCIATION(info->d, info->in,
		//					   ueId, ueIf,
		//					   true);
		info = info->next;
	}
}

#pragma endregion

#pragma region PDCPENTITY
static ptrLTENR_PDCPENTITY LTENR_PDCP_CreateNewEntity(NETSIM_ID enbId,
	NETSIM_ID enbIf,
	NETSIM_ID ueId,
	NETSIM_ID ueIf,
	bool isControlPacket)
{
	ptrLTENR_PDCPENTITY entity = calloc(1, sizeof * entity);
	entity->enbId = enbId;
	entity->enbIf = enbIf;
	entity->ueId = ueId;
	entity->ueIf = ueIf;
	entity->isControlPacket = isControlPacket;

	//State variable
	entity->RX_NEXT = 0;
	entity->TX_NEXT = 0;

	//Init buffer
	entity->transmissionBuffer = LTENR_BUFFER_INIT(INFINITE_BUFFER);
	entity->receiveBuffer = LTENR_BUFFER_INIT(INFINITE_BUFFER);

	return entity;
}

static void LTENR_PDCP_AddEntity(ptrLTENR_PDCPVAR pdcp,
	ptrLTENR_PDCPENTITY entity)
{
	ptrLTENR_PDCPENTITY e = pdcp->pdcpEntity;
	if (!e)
	{
		pdcp->pdcpEntity = entity;
		return;
	}

	while (e->next)
		e = e->next;
	e->next = entity;
}

static void LTENR_PDCP_FreeEntity(ptrLTENR_PDCPENTITY entity)
{
	LTENR_BUFFER_DELETE(entity->transmissionBuffer);
	LTENR_BUFFER_DELETE(entity->receiveBuffer);
	free(entity);
	entity->receiveBuffer = NULL;
	entity->transmissionBuffer = NULL;
}

static void LTENR_PDCP_RemoveEntity(ptrLTENR_PDCPVAR pdcp,
	ptrLTENR_PDCPENTITY entity)
{
	ptrLTENR_PDCPENTITY e, p = NULL;
	e = pdcp->pdcpEntity;
	while (e)
	{
		if (e == entity)
		{
			if (!p)
			{
				pdcp->pdcpEntity = e->next;
				e->isDiscardTimerStarted = false;
				fnDeleteEvent(e->discardTimerId);
				e->isT_ReorderingRunning = false;
				fnDeleteEvent(e->t_ReorderingEventId);
				LTENR_PDCP_FreeEntity(e);
				return;
			}
			else
			{
				p->next = e->next;
				e->isDiscardTimerStarted = false;
				fnDeleteEvent(e->discardTimerId);
				e->isT_ReorderingRunning = false;
				fnDeleteEvent(e->t_ReorderingEventId);
				LTENR_PDCP_FreeEntity(e);
				return;
			}
		}
		p = e;
		e = e->next;
	}
}

ptrLTENR_PDCPENTITY LTENR_PDCP_FindEntity(ptrLTENR_PDCPVAR pdcp,
	NETSIM_ID enbId,
	NETSIM_ID enbIf,
	NETSIM_ID ueId,
	NETSIM_ID ueIf,
	bool isControlPacket)
{
	ptrLTENR_PDCPENTITY e = pdcp->pdcpEntity;
	while (e)
	{
		if (e->enbId == enbId &&
			e->enbIf == enbIf &&
			e->ueId == ueId &&
			e->ueIf == ueIf &&
			e->isControlPacket == isControlPacket)
			return e;
		e = e->next;
	}
	return NULL;
}

static ptrLTENR_PDCPENTITY LTENR_PDCPENTITY_GET(ptrLTENR_PDCPVAR pdcp,
	NETSIM_ID d,
	NETSIM_ID in,
	NETSIM_ID remote,
	bool isControlPacket)
{
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, remote);

	if (!rin)
	{
		fnNetSimError("Device %d is not associated with device %d.\n",
			d, remote);
		return NULL;
	}

	if (isUE(d, in))
		return LTENR_PDCP_FindEntity(pdcp, remote, rin, d, in, isControlPacket);
	else if (isGNB(d, in))
		return LTENR_PDCP_FindEntity(pdcp, d, in, remote, rin, isControlPacket);
	else
	{
		fnNetSimError("%d device is not an UE or gNB in function %s\n",
			d, __FUNCTION__);
		return NULL;
	}
}

static void LTENR_GNBPDCP_CreatePDCPEntityForUE(NETSIM_ID gnbId,
	NETSIM_ID gnbIf,
	NETSIM_ID ueId,
	NETSIM_ID ueIf,
	bool isControlPacket)
{
	ptrLTENR_PDCPVAR pdcp = LTENR_GNBPDCP_GET(gnbId, gnbIf);
	ptrLTENR_PDCPENTITY entity = LTENR_PDCP_CreateNewEntity(gnbId, gnbIf, ueId, ueIf, isControlPacket);
	LTENR_PDCP_AddEntity(pdcp, entity);
	entity->discardDelayTime = pdcp->discardDelayTimer * MILLISECOND;
	entity->t_Reordering = pdcp->t_Reordering * MILLISECOND;
}

static void LTENR_UEPDCP_CreatePDCPEntity(NETSIM_ID gnbId,
	NETSIM_ID gnbIf,
	NETSIM_ID ueId,
	NETSIM_ID ueIf,
	bool isControlPacket)
{
	ptrLTENR_PDCPVAR pdcp = LTENR_UEPDCP_GET(ueId, ueIf);
	ptrLTENR_PDCPENTITY entity = LTENR_PDCP_CreateNewEntity(gnbId, gnbIf, ueId, ueIf, isControlPacket);
	LTENR_PDCP_AddEntity(pdcp, entity);
	entity->discardDelayTime = pdcp->discardDelayTimer * MILLISECOND;
	entity->t_Reordering = pdcp->t_Reordering * MILLISECOND;
}

#pragma endregion

#pragma region PDCPHDR

static void LTENR_PDCP_ADDHDR(UINT32 sn,
	NetSim_PACKET* packet)
{
	ptrLTENR_PDCP_HDR hdr = calloc(1, sizeof * hdr);
	hdr->dcBit = DCBIT_DATA;
	hdr->SN = sn % POW_2_SN_SIZE;
	LTENR_HDR_ADD_TO_PACKET(packet, hdr, PDCPHDRID, LTENR_PDCP_HDR_LEN);
	print_ltenr_log("Adding PDCP hdr to packet. Hdr Size = %d\n", LTENR_PDCP_HDR_LEN);
	print_ltenr_log("\tDCBIT = %d\n", hdr->dcBit);
	print_ltenr_log("\tSN = %d\n", hdr->SN);
}

ptrLTENR_PDCP_HDR LTENR_PDCP_HDR_COPY(ptrLTENR_PDCP_HDR hdr)
{
	ptrLTENR_PDCP_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	return ret;
}

void LTENR_PDCP_HDR_FREE(ptrLTENR_PDCP_HDR hdr)
{
	free(hdr);
}

void LTENR_PDCP_HDR_Trace(ptrLTENR_PDCP_HDR hdr, char* s)
{
	char info[BUFSIZ];

	sprintf(info, "Hdr Type = %s\n"
		"SN=%d\n dcBit=%d\n",
		"PDCP_HDR", hdr->SN, hdr->dcBit);
	strcat(s, info);
}

#pragma endregion

#pragma region PDCP_HDRCOMPRESSION
#pragma warning(disable:4100)
void LTENR_PDCP_HDRCOMPRESSION(NetSim_PACKET* packet)
{
	// Implementation of ROHC is out of scope for NetSim.
	// An user can write their own code here
}
#pragma warning(default:4100)
#pragma endregion

#pragma region PDCP_DISCARDTIMER

void LTENR_PDCP_START_DISCARD_TIMER(ptrLTENR_PDCPENTITY entity)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime + entity->discardDelayTime + 1;
	pevent.nDeviceId = pstruEventDetails->nDeviceId;
	pevent.nDeviceType = pstruEventDetails->nDeviceType;
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = pstruEventDetails->nInterfaceId;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PDCP_DISCARDTIMER);
	pevent.szOtherDetails = entity;
	entity->discardTimerId = fnpAddEvent(&pevent);
	entity->isDiscardTimerStarted = true;
	print_ltenr_log("Starting Discard timer. Expires at %lfus\n", pevent.dEventTime);
}

void LTENR_PDCP_RESET_DISCARD_TIMER(ptrLTENR_PDCPENTITY entity)
{
	if (!LTENR_BUFFER_HASPACKET(entity->transmissionBuffer))
	{
		// No packet in transmission buffer. Turn off discard timer.
		entity->discardTimerId = 0;
		entity->isDiscardTimerStarted = false;
		return;
	}

	double time;
#pragma warning(disable:4189)
	NetSim_PACKET* packet = LTENR_BUFFER_HEADPACKET(entity->transmissionBuffer,
		NULL,
		&time,
		NULL);
#pragma warning(default:4189)
	NetSim_EVENTDETAILS pevent;
	memcpy(&pevent, pstruEventDetails, sizeof pevent);
	pevent.dEventTime = time + entity->discardDelayTime + 1;
	pevent.szOtherDetails = entity;
	entity->discardTimerId = fnpAddEvent(&pevent);
	entity->isDiscardTimerStarted = true;
}

void LTENR_PDCP_HANDLE_DISCARD_TIMER()
{
	double currTime = pstruEventDetails->dEventTime;
	ptrLTENR_PDCPENTITY entity = pstruEventDetails->szOtherDetails;
	double time;
	bool isMore = true;
	while (isMore)
	{
		NetSim_PACKET* packet = LTENR_BUFFER_HEADPACKET(entity->transmissionBuffer,
			NULL,
			&time,
			NULL);
		if (entity->discardDelayTime + time <= currTime)
		{
			packet = LTENR_BUFFER_GETPACKET(entity->transmissionBuffer,
				NULL, NULL, NULL);
			fn_NetSim_Packet_FreePacket(packet);
			isMore = LTENR_BUFFER_HASPACKET(entity->transmissionBuffer);
		}
		else
		{
			isMore = false;
		}
	}

	LTENR_PDCP_RESET_DISCARD_TIMER(entity);
}

#pragma endregion

#pragma region PDCP_TREORDERING_TIMER
static void LTENR_PDCP_START_TREORDERING(ptrLTENR_PDCPENTITY entity)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime + entity->t_Reordering;
	pevent.nDeviceId = pstruEventDetails->nDeviceId;
	pevent.nDeviceType = pstruEventDetails->nDeviceType;
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = pstruEventDetails->nInterfaceId;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PDCP_TREORDERINGTIMER);
	pevent.szOtherDetails = entity;
	entity->t_ReorderingEventId = fnpAddEvent(&pevent);
	entity->isT_ReorderingRunning = true;
}

static void LTENR_PDCP_STOP_TREORDERING(ptrLTENR_PDCPENTITY entity)
{
	fnDeleteEvent(entity->t_ReorderingEventId);
	entity->isT_ReorderingRunning = false;
}

static void LTENR_PDCP_ACTION_FOR_TREORDERING_ONRECEV(ptrLTENR_PDCPENTITY entity)
{
	if (entity->isT_ReorderingRunning &&
		entity->RX_DELIV >= entity->RX_REORD)
	{
		LTENR_PDCP_STOP_TREORDERING(entity);
		LTENR_PDCP_START_TREORDERING(entity);
	}
	else if (!entity->isT_ReorderingRunning &&
		entity->RX_DELIV < entity->RX_REORD)
	{
		entity->RX_REORD = entity->RX_NEXT;
		LTENR_PDCP_START_TREORDERING(entity);
	}
}

void LTENR_PDCP_HANDLE_TREORDERINGTIMER()
{
	ptrLTENR_PDCPENTITY entity = pstruEventDetails->szOtherDetails;
	entity->isT_ReorderingRunning = false;
	entity->t_ReorderingEventId = 0;

	ptrLTENR_PDCPRXBUFFER_OPTIONAL opt;
	UINT32 next = 0;
	while (LTENR_BUFFER_HASPACKET(entity->receiveBuffer))
	{
		NetSim_PACKET* packet = LTENR_BUFFER_HEADPACKET(entity->receiveBuffer, NULL, NULL, &opt);
		if (!next)
			next = opt->count;
		if (next == opt->count)
		{
			packet = LTENR_BUFFER_GETPACKET(entity->receiveBuffer, NULL, NULL, &opt);
			LTENR_HDR_FREE_FROM_PACKET(packet, PDCPHDRID);
			free(opt);
			pstruEventDetails->pPacket = packet;
			LTENR_CallRRCIn();
			next++;
		}
		else
		{
			entity->RX_DELIV = opt->count;
			break;
		}
	}
	if (entity->RX_DELIV < entity->RX_NEXT)
	{
		entity->RX_REORD = entity->RX_NEXT;
		LTENR_PDCP_START_TREORDERING(entity);
	}
}
#pragma endregion

#pragma region PDCP_BUFFER
static void LTENR_PDCP_TXBUFFER_ENQUEUE(ptrLTENR_PDCPENTITY entity,
	NetSim_PACKET* packet)
{
	LTENR_BUFFER_ADDPACKET(entity->transmissionBuffer,
		fn_NetSim_Packet_CopyPacket(packet),
		(UINT64)packet->pstruMacData->dPacketSize,
		pstruEventDetails->dEventTime,
		NULL);
	if (!entity->isDiscardTimerStarted)
		LTENR_PDCP_START_DISCARD_TIMER(entity);
}

bool LTENR_PDCP_RXBUFFER_SORT(ptrLTENR_BUFFEREDPACKET cbp,
	ptrLTENR_BUFFEREDPACKET nbp)
{
	ptrLTENR_PDCPRXBUFFER_OPTIONAL co = cbp->optionalInfo;
	ptrLTENR_PDCPRXBUFFER_OPTIONAL no = nbp->optionalInfo;
	if (co->count > no->count)
		return true;
	return false;
}

static void LTENR_PDCP_RXBUFFER_ENQUEUE(ptrLTENR_PDCPENTITY entity,
	NetSim_PACKET* packet,
	UINT32 recv_count)
{
	ptrLTENR_PDCPRXBUFFER_OPTIONAL opt = calloc(1, sizeof * opt);
	opt->count = recv_count;
	LTENR_BUFFER_ADDPACKET_SORTED(entity->receiveBuffer,
		packet,
		(UINT64)packet->pstruMacData->dPacketSize,
		pstruEventDetails->dEventTime,
		opt,
		LTENR_PDCP_RXBUFFER_SORT);
	entity->RX_DELIV = ((ptrLTENR_PDCPRXBUFFER_OPTIONAL)entity->receiveBuffer->headPacket->optionalInfo)->count;
}
#pragma endregion

#pragma region PDCP_PACKET_PROCESSING

static UINT32 LTENR_PDCP_SET_SN(ptrLTENR_PDCPENTITY entity)
{

	UINT32 sn = entity->TX_NEXT;
	entity->TX_NEXT++;
	return sn;
}

static void LTENR_PDCP_CALCULATE_STATE_VARIABLE(ptrLTENR_PDCPENTITY entity,
	ptrLTENR_PDCP_HDR hdr,
	PUINT32 recv_sn,
	PUINT32 recv_hfn,
	PUINT32 count)
{
	UINT32 delvSN = LTENR_PDCP_SN(entity->RX_DELIV);
	UINT32 delvHFN = LTENR_PDCP_HFN(entity->RX_DELIV);
	*recv_sn = hdr->SN;

	if ((int)hdr->SN < ((int)(delvSN - LTENR_PDCP_WINDOW_SIZE)))
	{
		*recv_hfn = delvHFN + 1;
	}
	else if ((int)hdr->SN >= ((int)(delvSN + LTENR_PDCP_WINDOW_SIZE)))
	{
		*recv_hfn = delvHFN - 1;
	}
	else
	{
		*recv_hfn = delvHFN;
	}

	*count = LTENR_PDCP_COUNT(*recv_hfn, *recv_sn);

}

static void LTENR_PDCP_UPDATE_STATE_VARIABLE(ptrLTENR_PDCPENTITY entity,
	UINT32 recv_count)
{
	if (recv_count >= entity->RX_NEXT)
		entity->RX_NEXT = recv_count + 1;
}

static void LTENR_PDCP_SendUp()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	if (LTENR_IS_UPLANE_MSG(packet))
		LTENR_CallSDAPIn();
	else {
		switch (pstruEventDetails->pPacket->nControlDataType)
		{
		case LTENR_MSG_NAS_HANDOVER_COMMAND:
			LTENR_CallNASIn();
			break;
		default:
			LTENR_CallRRCIn();
			break;
		}
	}
}

static void LTENR_PDCP_DeliverSDUToUpperLayer(ptrLTENR_PDCPVAR pdcp, ptrLTENR_PDCPENTITY entity,
	UINT32 recv_count)
{
	ptrLTENR_PDCPRXBUFFER_OPTIONAL opt;
	if (pdcp->isoutOfOrderDelivery)
	{
		NetSim_PACKET* packet = LTENR_BUFFER_GETPACKET(entity->receiveBuffer, NULL, NULL, &opt);
		LTENR_HDR_FREE_FROM_PACKET(packet, PDCPHDRID);
		free(opt);
		pstruEventDetails->pPacket = packet;
		LTENR_PDCP_SendUp();
		return;
	}

	if (recv_count == entity->RX_DELIV)
	{
		UINT32 next = 0;
		while (LTENR_BUFFER_HASPACKET(entity->receiveBuffer))
		{
			NetSim_PACKET* packet = LTENR_BUFFER_HEADPACKET(entity->receiveBuffer, NULL, NULL, &opt);
			if (!next)
				next = opt->count;
			if (next == opt->count)
			{
				packet = LTENR_BUFFER_GETPACKET(entity->receiveBuffer, NULL, NULL, &opt);
				LTENR_HDR_FREE_FROM_PACKET(packet, PDCPHDRID);
				free(opt);
				pstruEventDetails->pPacket = packet;
				LTENR_PDCP_SendUp();
				next++;
			}
			else
			{
				entity->RX_DELIV = opt->count;
				break;
			}
		}

	}
}

void fn_NetSim_LTENR_PDCP_TransmitPDCPSDU()
{
	LTENR_PDCP_PRINT_PACKET_LOG_OUT();
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	bool isControlPacket = false;

	if (!packet)
	{
		fnNetSimError("LTENR-PDCP %s function is called without packet\n",
			__FUNCTION__);
		return;
	}

	if (!packet->nReceiverId)
	{
		fnNetSimError("LTENR-PDCP %s function is called without valid receiver.\n"
			"Broadcast or multicast is not supported by LTE-NR in NetSim\n",
			__FUNCTION__);
		return;
	}

	if (packet->nPacketType == PacketType_Control)
		isControlPacket = true;

	ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(d, in);
	ptrLTENR_PDCPENTITY entity = LTENR_PDCPENTITY_GET(pdcp, d, in, packet->nReceiverId, isControlPacket);
	if (!entity)
	{
		fnNetSimError("No PDCP entity is found between device %d and %d\n"
			"Dropping packet in function %s\n",
			d, packet->nReceiverId, __FUNCTION__);
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}

	UINT32 sn = LTENR_PDCP_SET_SN(entity);

	print_ltenr_log("Enqueuing packet to transmission buffer.\n");
	LTENR_PDCP_TXBUFFER_ENQUEUE(entity, packet);

	if (LTENR_IS_UPLANE_MSG(packet))
		LTENR_PDCP_HDRCOMPRESSION(packet);

	LTENR_PDCP_ADDHDR(sn, packet);
	print_ltenr_log("Passing packet to RLC sublayer.\n");
	LTENR_CallRLCOut();
}

void fn_NetSim_LTENR_PDCP_ReceivePDCPSDU()
{
	LTENR_PDCP_PRINT_PACKET_LOG_IN();
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	bool isControlPacket = false;

	if (!packet)
	{
		fnNetSimError("LTENR-PDCP %s function is called without packet\n",
			__FUNCTION__);
		return;
	}
	if (packet->nPacketType == PacketType_Control)
		isControlPacket = true;

	ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(d, in);
	ptrLTENR_PDCPENTITY entity = LTENR_PDCPENTITY_GET(pdcp, d, in, packet->nTransmitterId, isControlPacket);
	if (!entity)
	{
		fnNetSimError("No PDCP entity is found between device %d and %d\n"
			"Dropping packet in function %s\n",
			d, packet->nTransmitterId
			, __FUNCTION__);
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}

	ptrLTENR_PDCP_HDR hdr = LTENR_HDR_GET_FROM_PACKET(packet, PDCPHDRID);
	if (!hdr)
	{
		fnNetSimError("No PDCP hdr is found in packet.\n"
			"Dropping packet in function %s\n", __FUNCTION__);
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;

		return;
	}

	UINT32 RECV_SN;
	UINT32 RECV_HFN;
	UINT32 RECV_COUNT;
	LTENR_PDCP_CALCULATE_STATE_VARIABLE(entity, hdr,
		&RECV_SN, &RECV_HFN, &RECV_COUNT);
	if (LTENR_PDCP_ISDUPLICATEPDU(entity, RECV_COUNT)
		&& packet->nPacketType != PacketType_Control) {
		// Discard the PDCP PDU
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}

	LTENR_PDCP_RXBUFFER_ENQUEUE(entity, packet, RECV_COUNT);
	LTENR_PDCP_UPDATE_STATE_VARIABLE(entity,
		RECV_COUNT);

	LTENR_PDCP_DeliverSDUToUpperLayer(pdcp, entity, RECV_COUNT);
	LTENR_PDCP_ACTION_FOR_TREORDERING_ONRECEV(entity);
}

#pragma endregion

#pragma region PDCP_ASSOCIATION

void LTENR_PDCP_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
	NETSIM_ID ueId, NETSIM_ID ueIf,
	bool isAssociated)
{
	if (!LTENR_GNBPDCP_ISINIT(gnbId, gnbIf))
		return; // gNB PDCP is still not initiallized.
	ptrLTENR_PDCPVAR gpdcp = LTENR_GNBPDCP_GET(gnbId, gnbIf);

	if (!LTENR_UEPDCP_ISINIT(ueId, ueIf))
		return; // UE PDCP is still not initiallized.
	ptrLTENR_PDCPVAR uepdcp = LTENR_UEPDCP_GET(ueId, ueIf);

	if (isAssociated)
	{
		ptrLTENR_PDCPENTITY entity = LTENR_PDCP_FindEntity(gpdcp, gnbId, gnbIf,
			ueId, ueIf, true);
		if (!entity)
			LTENR_GNBPDCP_CreatePDCPEntityForUE(gnbId, gnbIf,
				ueId, ueIf, true);

		entity = LTENR_PDCP_FindEntity(gpdcp, gnbId, gnbIf,
			ueId, ueIf, false);
		if (!entity)
			LTENR_GNBPDCP_CreatePDCPEntityForUE(gnbId, gnbIf,
				ueId, ueIf, false);

		entity = LTENR_PDCP_FindEntity(uepdcp, gnbId, gnbIf,
			ueId, ueIf, true);
		if (!entity)
			LTENR_UEPDCP_CreatePDCPEntity(gnbId, gnbIf,
				ueId, ueIf, true);

		entity = LTENR_PDCP_FindEntity(uepdcp, gnbId, gnbIf,
			ueId, ueIf, false);
		if (!entity)
			LTENR_UEPDCP_CreatePDCPEntity(gnbId, gnbIf,
				ueId, ueIf, false);

	}
	else
	{
		ptrLTENR_PDCPENTITY entity = LTENR_PDCP_FindEntity(gpdcp, gnbId, gnbIf,
			ueId, ueIf, true);
		LTENR_PDCP_RemoveEntity(gpdcp, entity);

		entity = LTENR_PDCP_FindEntity(gpdcp, gnbId, gnbIf,
			ueId, ueIf, false);
		LTENR_PDCP_RemoveEntity(gpdcp, entity);

		entity = LTENR_PDCP_FindEntity(uepdcp, gnbId, gnbIf,
			ueId, ueIf, true);
		LTENR_PDCP_RemoveEntity(uepdcp, entity);

		entity = LTENR_PDCP_FindEntity(uepdcp, gnbId, gnbIf,
			ueId, ueIf, false);
		LTENR_PDCP_RemoveEntity(uepdcp, entity);
	}
}

#pragma endregion
