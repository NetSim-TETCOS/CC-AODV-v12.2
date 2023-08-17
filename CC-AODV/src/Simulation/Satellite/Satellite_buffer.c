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
#include "main.h"
#include "SATELLITE.h"
#include "Satellite_MAC.h"

ptrSATELLITE_BUFFER satellite_buffer_init(NETSIM_ID utId, NETSIM_ID utIf,
										  NETSIM_ID gwId, NETSIM_ID gwIf,
										  double sizeInBytes, double maxUnitSizeInBytes)
{
	ptrSATELLITE_BUFFER buff = calloc(1, sizeof * buff);
	buff->utId = utId;
	buff->utIf = utIf;
	buff->gwId = gwId;
	buff->gwIf = gwIf;
	buff->maxSizeInBytes = sizeInBytes;
	buff->maxUnitSizeInBytes = maxUnitSizeInBytes;
	return buff;
}

void satellite_buffer_setMaxUnitSizeInBytes(ptrSATELLITE_BUFFER buffer, double maxUnitSizeInBytes)
{
	buffer->maxUnitSizeInBytes = maxUnitSizeInBytes;
}

bool satellite_buffer_add_packet(ptrSATELLITE_BUFFER buffer, NetSim_PACKET* packet)
{
	if (buffer->maxUnitSizeInBytes < packet->pstruMacData->dPayload)
	{
		fn_NetSim_Packet_FreePacket(packet);
		return false;
	}

	if (buffer->sizeInBytes + packet->pstruMacData->dPayload > buffer->maxSizeInBytes)
	{
		fn_NetSim_Packet_FreePacket(packet);
		return false;
	}

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

	buffer->count++;
	buffer->sizeInBytes += packet->pstruMacData->dPayload;
	return true;
}

NetSim_PACKET* satellite_buffer_remove_packet(ptrSATELLITE_BUFFER buffer)
{
	if (!buffer->head) return NULL;

	NetSim_PACKET* packet = buffer->head;
	buffer->head = packet->pstruNextPacket;
	packet->pstruNextPacket = NULL;

	if (!buffer->head) buffer->tail = NULL;

	buffer->sizeInBytes -= packet->pstruMacData->dPayload;
	buffer->count--;

	return packet;
}

NetSim_PACKET* satellite_buffer_head_packet(ptrSATELLITE_BUFFER buffer)
{
	return buffer->head;
}

#pragma region SLOT_ALLOCATION
static void calculate_slot_reqd_per_buffer(ptrSATELLITE_BUFFER buf, UINT bitsPerSlot)
{
	buf->slotReqd = 0;

	double bufbits = (UINT)buf->sizeInBytes * 8;
	buf->slotReqd = (UINT)ceil(bufbits / bitsPerSlot);
}

static void calculate_slot_reqd(ptrSUPERFRAME sf)
{
	UINT i;
	for (i = 0; i < sf->bufferCount; i++)
	{
		calculate_slot_reqd_per_buffer(sf->buffers[i], sf->slotConfig->bitsPerSlot);
		if (sf->linkType == LINKTYPE_FORWARD)
		{
			print_satellite_log("Slot reqd for Buffer %d:%d-->%d:%d is %d\n",
								sf->buffers[i]->gwId, sf->buffers[i]->gwIf,
								sf->buffers[i]->utId, sf->buffers[i]->utIf,
								sf->buffers[i]->slotReqd);
		}
		else
		{
			print_satellite_log("Slot reqd for Buffer %d:%d-->%d:%d is %d\n",
								sf->buffers[i]->utId, sf->buffers[i]->utIf,
								sf->buffers[i]->gwId, sf->buffers[i]->gwIf,
								sf->buffers[i]->slotReqd);

		}
	}
}

static void sort_buffer(UINT count, ptrSATELLITE_BUFFER* buffers)
{
	ptrSATELLITE_BUFFER temp = NULL;
	UINT i, j;
	for (i = 0; i < count; i++)
	{
		for (j = i; j < count; j++)
		{
			if (buffers[j]->rank > buffers[i]->rank)
			{
				temp = buffers[i];
				buffers[i] = buffers[j];
				buffers[j] = temp;
			}
		}
	}
}

static void init_buffer_rank(ptrSUPERFRAME sf)
{
	UINT i;
	for (i = 0; i < sf->bufferCount; i++)
	{
		if(sf->buffers[i]->slotReqd)
			sf->buffers[i]->rank += 1;
		sf->buffers[i]->allocatedSlotCount = 0;
	}
}

static void allocate_slot_count(ptrSUPERFRAME sf, ptrFRAME fr)
{
	UINT i;
	UINT slotCount = sf->frameConfig->slotCountInFrame;
	for (i = 0; i < sf->bufferCount; i++)
	{
		sf->buffers[i]->allocatedSlotCount = min(sf->buffers[i]->slotReqd, slotCount);
		slotCount -= sf->buffers[i]->allocatedSlotCount;
		if (slotCount == 0) break;
	}
}

static UINT calculate_slot_reqd_for_packet(double size, UINT bitsPerSlot)
{
	return (UINT)ceil(size * 8.0 / bitsPerSlot);
}

static void schedule_packet(ptrSUPERFRAME sf, ptrFRAME fr)
{
	UINT i;
	UINT slcount;
	for (i = 0; i < sf->bufferCount; i++)
	{
		ptrSATELLITE_BUFFER buf = sf->buffers[i];
		if (!buf->allocatedSlotCount) break;

		slcount = buf->allocatedSlotCount;
		NetSim_PACKET* h;
		while (buf->head)
		{
			h = buf->head;
			if (slcount == 0) break;

			UINT slreqd = calculate_slot_reqd_for_packet(h->pstruMacData->dPayload, sf->slotConfig->bitsPerSlot);
			if (slreqd > slcount) break;
			slcount -= slreqd;
			buf->head = h->pstruNextPacket;
			if (buf->head == NULL)buf->tail = NULL;
			buf->count--;
			buf->sizeInBytes -= h->pstruMacData->dPayload;
			h->pstruNextPacket = NULL;

			if (fr->head)
			{
				fr->tail->pstruNextPacket = h;
				fr->tail = h;
			}
			else
			{
				fr->head = h;
				fr->tail = h;
			}
		}
	}
}

static void update_rank(ptrSUPERFRAME sf,ptrFRAME fr)
{
	UINT i;
	UINT slotCount = sf->frameConfig->slotCountInFrame;
	for (i = 0; i < sf->bufferCount; i++)
	{
		ptrSATELLITE_BUFFER buf = sf->buffers[i];
		buf->rank -= ((buf->allocatedSlotCount * 1.0) / slotCount);
	}
}

void satellite_allocate_slot(NETSIM_ID d, NETSIM_ID in,
							 ptrSUPERFRAME sf, ptrFRAME fr)
{
	print_satellite_log("Starting slot allocation for Superframe %d, Frame %d, LinkType = %s\n",
						sf->superFrameId, fr->frameId, strLINKTYPE[sf->linkType]);
	satellite_log_add_tab();
	calculate_slot_reqd(sf);
	init_buffer_rank(sf);
	sort_buffer(sf->bufferCount, sf->buffers);
	allocate_slot_count(sf, fr);
	schedule_packet(sf, fr);
	update_rank(sf, fr);
	satellite_log_remove_tab();
}
#pragma endregion

