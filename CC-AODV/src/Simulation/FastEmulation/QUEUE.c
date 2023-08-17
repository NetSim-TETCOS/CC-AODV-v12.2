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
#include "FastEmulation.h"
#include "QUEUE.h"
#include "EmulationPacket.h"
#include "EmulationLib.h"
#include "NetSim_utility.h"

static UINT queueCount;
ptrQUEUE* queueList;

#pragma region QUEUE_INIT
static ptrQUEUE alloc_new_queue()
{
	ptrQUEUE q = calloc(1, sizeof * q);
	if (queueCount)
		queueList = realloc(queueList, ((UINT64)queueCount + 1) * sizeof * queueList);
	else
		queueList = calloc(1, sizeof * queueList);
	queueList[queueCount] = q;
	queueCount++;
	q->queueId = queueCount;
	
	char mu[100];
	sprintf(mu, "Queue_Mutex_%d", q->queueId);
	q->packetList.mutex = CreateMutexA(NULL, false, mu);
	
	sprintf(mu, "Queue_event_%d", q->queueId);
	q->packetList.bufferEvent = CreateEventA(NULL, true, true, mu);
	return q;
}

static void init_wired_p2p_queue_(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri,
								  NetSim_LINKS* link)
{
	ptrQUEUE q = alloc_new_queue();
	q->type = QTYPE_P2P;

	ptrP2PQUEUE p2pQueue = calloc(1, sizeof * p2pQueue);
	q->p2pQueue = p2pQueue;

	p2pQueue->txId = t;
	p2pQueue->txIf = ti;

	p2pQueue->rxId = r;
	p2pQueue->rxIf = ri;

	p2pQueue->dataRate_mbps = link->puniMedProp.pstruWiredLink.dDataRateUp;
	p2pQueue->fixedDelayNonQueuing_us = (UINT64)link->puniMedProp.pstruWiredLink.dPropagationDelayDown;

	p2pQueue->ber = link->puniMedProp.pstruWiredLink.dErrorRateUp;
}

static void init_wired_p2p_queue(NetSim_LINKS* link)
{
	init_wired_p2p_queue_(link->puniDevList.pstruP2P.nFirstDeviceId,
						  link->puniDevList.pstruP2P.nFirstInterfaceId,
						  link->puniDevList.pstruP2P.nSecondDeviceId,
						  link->puniDevList.pstruP2P.nSecondInterfaceId,
						  link);

	init_wired_p2p_queue_(link->puniDevList.pstruP2P.nSecondDeviceId,
						  link->puniDevList.pstruP2P.nSecondInterfaceId,
						  link->puniDevList.pstruP2P.nFirstDeviceId,
						  link->puniDevList.pstruP2P.nFirstInterfaceId,
						  link);
}

static void init_new_queue(NetSim_LINKS* link)
{
	switch (link->nLinkMedium)
	{
		case PHY_MEDIUM_WIRED:
		{
			switch (link->nLinkType)
			{
				case LinkType_P2P:
					init_wired_p2p_queue(link);
					break;
				default:
					fnNetSimError("Link type is not supported by fast emulation\n");
					break;
			}
		}
		break;
		default:
			fnNetSimError("Link mode is not supported by fast emulation");
			break;
	}
}

void fastemulation_init_queue()
{
	NETSIM_ID i;
	for (i = 0; i < NETWORK->nLinkCount; i++)
	{
		NetSim_LINKS* link = NETWORK->ppstruNetSimLinks[i];
		if (link->nLinkMedium != PHY_MEDIUM_WIRED)
		{
			fnNetSimError("Only wired link mode is supported by fast emulation\n");
			continue;
		}

		if(link->nLinkType != LinkType_P2P)
		{
			fnNetSimError("Only P2P link type is supported by fast emulation\n");
			continue;
		}

		init_new_queue(link);
	}
}
#pragma endregion

ptrQUEUE find_queue(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri)
{
	UINT i;
	for (i = 0; i < queueCount; i++)
	{
		ptrQUEUE q = queueList[i];
		if (q->p2pQueue->txId == t &&
			q->p2pQueue->txIf == ti &&
			q->p2pQueue->rxId == r &&
			q->p2pQueue->rxIf == ri)
			return q;
	}
	return NULL;
}

void add_packet_to_queue(ptrPACKET p, UINT64 time)
{
	ptrQUEUE q = p->hops[p->currentHop];
	
	WaitForSingleObject(q->packetList.mutex, INFINITE);

	time = max(time, q->p2pQueue->lastSentTimeQueuing);
	time += (UINT64)((p->realPacket->realPacketLen * 8.0) / q->p2pQueue->dataRate_mbps);
	time += q->p2pQueue->fixedDelayQueuing_us;

	q->p2pQueue->lastSentTimeQueuing = time;

	p->reinjectTime = time + q->p2pQueue->fixedDelayNonQueuing_us;

	if (q->packetList.head)
	{
		q->packetList.tail->next = p;
		q->packetList.tail = p;
	}
	else
	{
		q->packetList.head = p;
		q->packetList.tail = p;
	}
	SetEvent(q->packetList.bufferEvent);

	ReleaseMutex(q->packetList.mutex);
}

ptrPACKET get_packet_from_queue(ptrQUEUE q)
{
	ptrPACKET p = NULL;

	WaitForSingleObject(q->packetList.mutex, INFINITE);
	if (q->packetList.head)
	{
		p = q->packetList.head;

		q->packetList.head = p->next;
		p->next = NULL;

		if (q->packetList.head == NULL) q->packetList.tail = NULL;
	}
	ReleaseMutex(q->packetList.mutex);
	return p;
}

#pragma region QUEUE_EVENT
DWORD WINAPI run_queue(ptrQUEUE* lpParam)
{
	WaitForSingleObject(fastEmulationSyncMutex, INFINITE);
	ReleaseMutex(fastEmulationSyncMutex);

	ptrQUEUE q = *lpParam;
	fprintf(stderr, "Starting thread for queue %d (%d:%d--%d:%d)\n",
			q->queueId,
			q->p2pQueue->txId, q->p2pQueue->txIf,
			q->p2pQueue->rxId, q->p2pQueue->rxIf);

	ptrPACKET p = NULL;

	while (isContinueEmulation)
	{
		if (!p)
		{
			p = get_packet_from_queue(q);
			if (!p)
			{
				//usleep(1);
				WaitForSingleObject(q->packetList.bufferEvent, INFINITE);
				ResetEvent(q->packetList.bufferEvent);
				continue;
			}
		}

		UINT64 t = em_current_time();
		UINT64 t1 = p->reinjectTime;

		if (t >= t1)
		{
			if (p->pathLen - 1 == p->currentHop)
			{
				reinject_pkt(p);
				free_packet(p);
			}
			else
			{
				p->currentHop++;
				add_packet_to_queue(p, t1);
			}
			p = NULL;
		}
		else
		{
			//usleep(t1 - t);
		}
	}
	return 0;
}

void create_queue_thread()
{
	UINT i;
	for (i = 0; i < queueCount; i++)
	{
		ptrQUEUE q = queueList[i];
		if (q->isQueueUsed)
		{
			q->queueEventHandler = CreateThread(NULL, 0, run_queue, &queueList[i], 0, &q->queueEventHandlerId);
		}
	}
}
#pragma endregion
