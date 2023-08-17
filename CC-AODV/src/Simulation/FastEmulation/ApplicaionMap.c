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
#include "ApplicationMap.h"
#include "../Application/Application.h"
#include "EmulationPacket.h"
#include "NetSim_utility.h"
#include "QUEUE.h"

static ptrAPPMAP* appMap;
static UINT appMapCount;

#pragma region APPMAP_INIT

static ptrAPPMAP alloc_new_map()
{
	if (appMapCount)
		appMap = realloc(appMap, ((UINT64)appMapCount + 1) * sizeof * appMap);
	else
		appMap = calloc(1, sizeof * appMap);

	ptrAPPMAP map = calloc(1, sizeof * map);
	appMap[appMapCount] = map;
	appMapCount++;

	char name[BUFSIZ];
	sprintf(name, "appmap_mutex_%d", appMapCount);
	map->mutex = CreateMutexA(NULL, true, name);

	return map;
}

static void add_new_app_map(ptrAPPLICATION_INFO info)
{
	ptrAPPMAP m = alloc_new_map();

	m->appId = info->id;
	m->simDestId = info->destList[0];
	m->simSrcId = info->sourceList[0];
	m->instanceCount = 1;
	m->instanceId = 1;

	APP_EMULATION_INFO* einfo = info->appData;
	m->destIP = IP_COPY(einfo->realDestIP);
	m->destPort = (UINT16)einfo->nDestinationPort;
	m->srcIP = IP_COPY(einfo->realSourceIP[0]);
	m->srcPort = (UINT16)einfo->nSourcePort;
	m->protocol = _strdup(einfo->protocol);
	m->filterString = _strdup(einfo->filterString);
}

void fastemulation_init_app_map()
{
	NETSIM_ID i;
	for (i = 0; i < NETWORK->nApplicationCount; i++)
	{
		ptrAPPLICATION_INFO info = NETWORK->appInfo[i];

		if (info->nAppType != TRAFFIC_EMULATION)
		{
			fnNetSimError("Fast emulation only support emulation application.\n");
			continue;
		}

		if (info->nTransmissionType != UNICAST)
		{
			fnNetSimError("Fast emulation only support Unicast application.\n");
			continue;
		}

		add_new_app_map(info);
	}
}

ptrAPPMAP fastemulation_init_new_instance_for_app(ptrAPPMAP map)
{
	ptrAPPMAP m = alloc_new_map();

	m->instanceId = map->instanceCount + 1;
	map->instanceCount++;

	m->appId = map->appId;
	m->simDestId = map->simDestId;
	m->simSrcId = map->simSrcId;

	m->destIP = IP_COPY(map->destIP);
	m->destPort = map->destPort;
	m->srcIP = IP_COPY(map->srcIP);
	m->srcPort = map->srcPort;
	m->protocol = _strdup(map->protocol);
	m->filterString = _strdup(map->filterString);
	return m;
}

void add_appmap_event()
{
	UINT i;
	for (i = 0; i < appMapCount; i++)
	{
		ptrAPPMAP map = appMap[i];

		EVENTDETAILS e;
		e.appMap = map;
		e.time = map->appStartTime;
		e.type = EVENTTYPE_APPSTART;
		add_event(&e);

		e.appMap = map;
		e.time = map->appEndTime;
		e.type = EVENTTYPE_APPEND;
		add_event(&e);
	}
}
#pragma endregion

#pragma region APP_PATH
static void setup_path_to_packet(ptrPACKET p, ptrAPPMAP map)
{
	p->pathLen = map->hopCount;
	p->hops = map->hops;
	p->currentHop = 0;
}

static bool isTwoAppInterfering(ptrAPPMAP m1, ptrAPPMAP m2)
{
	UINT i;
	UINT j;

	for (i = 0; i < m1->hopCount; i++)
	{
		for (j = 0; j < m2->hopCount; j++)
		{
			if (m1->hops[i] == m2->hops[j]) return true;
		}
	}

	return false;
}

static void check_for_interfering_app(ptrAPPMAP map)
{
	UINT i;
	for (i = 0; i < appMapCount; i++)
	{
		if (map == appMap[i]) continue;

		if (map->appId == appMap[i]->appId) continue;

		if (isTwoAppInterfering(map, appMap[i])) return;
	}
	map->isNonInterferingApp = true;
}

static double find_min_rate(ptrAPPMAP map, UINT* id)
{
	double m = map->hops[0]->p2pQueue->dataRate_mbps;
	*id = 0;
	UINT i;
	for (i = 0; i < map->hopCount; i++)
	{
		ptrQUEUE q = map->hops[i];
		if (m > q->p2pQueue->dataRate_mbps)
		{
			m = q->p2pQueue->dataRate_mbps;
			*id = i;
		}
	}
	return m;
}

static void appmap_update_parameters(ptrAPPMAP map)
{
	if (!map->hopCount) return;

	UINT id;
	map->queuingRate = find_min_rate(map, &id);
	double nonQueuingRate = 0;
	UINT i;
	for (i = 0; i < map->hopCount; i++)
	{
		ptrQUEUE q = map->hops[i];
		map->totalFixedDelay += q->p2pQueue->fixedDelayNonQueuing_us;

		if (i == id) continue;

		nonQueuingRate += 1.0 / q->p2pQueue->dataRate_mbps;

	}
	map->nonQueuingRate = (1.0 / nonQueuingRate);
}

static void mark_used_queue(ptrAPPMAP map)
{
	UINT i;
	for (i = 0; i < map->hopCount; i++)
		map->hops[i]->isQueueUsed = true;
}

void fastemulation_setup_non_interfering_app()
{
	UINT i;
	for (i = 0; i < appMapCount; i++)
	{
		check_for_interfering_app(appMap[i]);

		if (appMap[i]->isNonInterferingApp)
			appmap_update_parameters(appMap[i]);
		else
			mark_used_queue(appMap[i]);
	}
}
#pragma endregion

#pragma region MAP_PACKET
void dispatch_to_emulation(ptrREALPACKET packet, ptrAPPMAP map)
{
	UINT64 time = em_current_time();
	ptrPACKET p = create_new_packet(packet, time);
	p->appId = map->appId;

	metric_packet_capture(p->appId, p->realPacket->realPacketCount);

	errormodel_check_packet_for_error(packet, map);
	if (map->isNonInterferingApp)
	{
		UINT64 ls = max(map->lastReinjectTime, time);
		map->lastReinjectTime = (UINT64)(p->realPacket->realPacketLen * 8.0 / map->queuingRate) + ls;
		p->reinjectTime = map->lastReinjectTime + map->totalFixedDelay +
			(UINT64)(p->realPacket->realPacketLen * 8.0 / map->nonQueuingRate);
		add_packet_to_app_map(map, p);
	}
	else
	{
		setup_path_to_packet(p, map);
		add_packet_to_queue(p, time);
	}
}
#pragma endregion

#pragma region API
char* appmap_get_filter_string(UINT id)
{
	return appMap[id - 1]->filterString;
}

UINT appmap_get_map_count()
{
	return appMapCount;
}

ptrAPPMAP appmap_get_map(UINT id)
{
	return appMap[id - 1];
}

ptrAPPMAP appmap_find(NETSIM_ID appId, UINT instanceId)
{
	UINT i;
	for (i = 0; i < appMapCount; i++)
	{
		if (appMap[i]->appId == appId &&
			(instanceId == 0 || appMap[i]->instanceId == instanceId))
			return appMap[i];
	}
	return NULL;
}

void start_application(ptrAPPMAP map)
{
	map->isAppStarted = true;
	ReleaseMutex(map->mutex);
}

void close_application(ptrAPPMAP map)
{
	map->isAppStarted = false;
}
#pragma endregion
