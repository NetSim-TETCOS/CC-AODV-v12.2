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
#include "main.h"
#include "EmulationLib.h"
#include "FastEmulation.h"
#include "EmulationPacket.h"
#include "ApplicationMap.h"

static UINT64 packetId = 0;

ptrPACKET create_new_packet(ptrREALPACKET packet, UINT64 time)
{
	ptrPACKET p = calloc(1, sizeof * p);
	p->realPacket = packet;
	p->packetId = ++packetId;
	p->captureTime = time;
	return p;
}

void free_packet(ptrPACKET packet)
{
	free(packet->realPacket->realPacket);
	free(packet->realPacket);
	free(packet);
}

void add_packet_to_app_map(ptrAPPMAP map, ptrPACKET packet)
{
	WaitForSingleObject(map->queueMutex, INFINITE);
	if (map->headPacket)
	{
		ptrPACKET tail = map->tailPacket;
		tail->next = packet;
		map->tailPacket = packet;
	}
	else
	{
		map->headPacket = packet;
		map->tailPacket = packet;
	}
	ReleaseMutex(map->queueMutex);
}

ptrPACKET get_packet_from_app_map(ptrAPPMAP map)
{
	ptrPACKET ret = NULL;
	WaitForSingleObject(map->queueMutex, INFINITE);
	if (map->headPacket)
	{
		ret = map->headPacket;
		map->headPacket = ret->next;
		ret->next = NULL;

		if (!map->headPacket)
			map->tailPacket = NULL;
	}
	ReleaseMutex(map->queueMutex);
	return ret;
}

ptrPACKET get_head_packet_ptr_from_app_map(ptrAPPMAP map)
{
	ptrPACKET ret = NULL;
	WaitForSingleObject(map->queueMutex, INFINITE);
	if (map->headPacket)
	{
		ret = map->headPacket;
	}
	ReleaseMutex(map->queueMutex);
	return ret;
}
