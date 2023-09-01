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
#include "NetSim_utility.h"
#include "FastEmulation.h"
#include "ApplicationMap.h"
#include "QUEUE.h"

static UINT64 read_time(char* s)
{
	char* t = strtok(s, "=");
	t = strtok(NULL, "=");

	char* p = lskip(t);
	p = rstrip(t);

	return (UINT64)(atof(p)*SECOND);
}

static char** find_all_hops(char* s, UINT* count)
{
	char** ret = NULL;
	UINT c = 0;
	char* t = strtok(s, ";");
	while (t)
	{
		if (ret)
			ret = realloc(ret, ((size_t)c + 1) * sizeof * ret);
		else
			ret = calloc(1, sizeof * ret);

		ret[c] = t;
		c++;

		t = strtok(NULL, ";");
	}
	*count = c;
	return ret;
}

static void add_queue_to_path(ptrAPPMAP map, ptrQUEUE q)
{
	if (map->hopCount)
		map->hops = realloc(map->hops, ((size_t)map->hopCount + 1) * sizeof * map->hops);
	else
		map->hops = calloc(1, sizeof * map->hops);

	map->hops[map->hopCount] = q;
	map->hopCount++;

	map->ber += q->p2pQueue->ber;
}

static void set_path_for_app(ptrAPPMAP map, char* path)
{
	UINT hopCount = 0;
	char** hops = NULL;

	if (*path == 'n' || *path == 'N')
		return;

	hops = find_all_hops(path, &hopCount);

	UINT i;
	for (i = 0; i < hopCount - 1; i++)
	{
		char s[50];
		char s1[50];
		strcpy(s, hops[i]);
		strcpy(s1, hops[i + 1]);

		char* t = strtok(s, ":");
		NETSIM_ID tx = atoi(t);
		t = strtok(NULL, ":");
		NETSIM_ID ti = atoi(t);

		t = strtok(s1, ":");
		NETSIM_ID rx = atoi(t);
		t = strtok(NULL, ":");
		NETSIM_ID ri = atoi(t);

		if (tx == rx) continue;

		ptrQUEUE q = find_queue(tx, ti, rx, ri);

		if (!q)
		{
			fnNetSimError("No link is found between %d:%d-%d:%d.\n",
						  tx, ti, rx, ri);
		}

		add_queue_to_path(map, q);
	}
}

static void add_path_to_app(char* path, NETSIM_ID appId, UINT64 time)
{
	ptrAPPMAP map = appmap_find(appId, 0);
	set_path_for_app(map, path);
	map->appStartTime = time;
	map->appEndTime = 0xFFFFFFFFFFFF;
}

static void add_path_to_new_app_instance(char* path, char* prevPath, NETSIM_ID appId, UINT64 time)
{
	if (!_stricmp(path, prevPath)) return;

	ptrAPPMAP headMap = appmap_find(appId, 0);

	ptrAPPMAP map = fastemulation_init_new_instance_for_app(headMap);

	ptrAPPMAP prevMap = appmap_find(appId, map->instanceId - 1);
	prevMap->appEndTime = time;

	map->appStartTime = time;
	map->appEndTime = 0xFFFFFFFFFFFF;

	set_path_for_app(map, path);
}

void fastemulation_setup_path()
{
	UINT64 currTime = 0;
	char** prevPath;
	NETSIM_ID i;
	NETSIM_ID appId = 0;

	prevPath = calloc(NETWORK->nApplicationCount, sizeof * prevPath);
	for (i = 0; i < NETWORK->nApplicationCount; i++)
		prevPath[i] = calloc(BUFSIZ, sizeof * prevPath[i]);

	char routeFile[BUFSIZ];
	sprintf(routeFile, "%s/%s", pszIOPath, "FastEmulation.route");
	FILE* fp = fopen(routeFile, "r");
	if (!fp)
	{
		fnSystemError("Unable to open FastEmulation.route file\n");
		perror(routeFile);
		return;
	}

	char s[BUFSIZ];
	while (fgets(s, BUFSIZ, fp))
	{
		char* t = s;
		t = lskip(t);
		t = rstrip(t);

		if (*t == '\0') continue; //empty line
		if (*t == '#') continue; //Comment line

		if (*t == 't' || *t == 'T')
		{
			//Time line
			currTime = read_time(t);
			appId = 0;
			continue;
		}

		appId++;

		char m[BUFSIZ];
		strcpy(m, t);
		if (prevPath[appId - 1][0] != 0)
			add_path_to_new_app_instance(t, prevPath[appId - 1], appId, currTime);
		else
			add_path_to_app(t, appId, currTime);

		strcpy(prevPath[appId - 1], m);
	}

	for (i = 0; i < NETWORK->nApplicationCount; i++)
		free(prevPath[i]);
	free(prevPath);
}
