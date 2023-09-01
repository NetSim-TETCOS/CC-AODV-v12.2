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

HANDLE metricThread;
DWORD metricThreadId;
CONSOLE_SCREEN_BUFFER_INFO cursorInfo;
HANDLE hConsole;

static UINT64 numTotalPacketCaptured;
static UINT64 numTotalPacketReinjected;
static UINT64 numTotalPacketErrored;

static UINT64* numPacketCaptured;
static UINT64* numPacketReinjected;
static UINT64* numPacketErrored;

DWORD WINAPI write_metric(LPVOID lpParam)
{
	WaitForSingleObject(fastEmulationSyncMutex, INFINITE);
	ReleaseMutex(fastEmulationSyncMutex);
	fprintf(stderr, "Starting thread for metrics\n");

	Sleep(5000);
	HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
	GetConsoleScreenBufferInfo(hConsole, &cursorInfo);

	UINT appCount = appmap_get_map_count();
	UINT i;
	while (isContinueEmulation)
	{
		SetConsoleCursorPosition(hConsole, cursorInfo.dwCursorPosition);
		fprintf(stderr, "Time = %lf second\n", em_current_time()/1000000.0);
		fprintf(stderr, "APP Id\tPacketCaptured\tPacketReinjected\tPacketErrored\n");
		for (i = 0; i < appCount; i++)
		{
			ptrAPPMAP map = appmap_get_map(i + 1);
			if (map->instanceId == 1)
				fprintf(stderr, "%d\t%lld\t\t%lld\t\t%lld\n",
					i + 1, numPacketCaptured[i], numPacketReinjected[i], numPacketErrored[i]);
		}
		fprintf(stderr, "Total\t%lld\t\t%lld\t\t%lld\n", numTotalPacketCaptured,
				numTotalPacketReinjected, numTotalPacketErrored);
		fprintf(stderr, "More Feature is comming soon...\n");

		Sleep(100);
	}
	char mfile[BUFSIZ];
	sprintf(mfile, "%s\\%s", pszIOPath, "FastEmulationMetrics.txt");
	FILE* fp = fopen(mfile, "w");
	fprintf(fp, "Time = %lf second\n", em_current_time() / 1000000.0);
	fprintf(fp, "App Id\tPacketCaptured\tPacketReinjected\tPacketErrored\n");
	for (i = 0; i < appCount; i++)
	{
		ptrAPPMAP map = appmap_get_map(i + 1);
		if(map->instanceId == 1)
			fprintf(fp, "%d\t%lld\t\t%lld\t\t%lld\n",
					i + 1, numPacketCaptured[i], numPacketReinjected[i], numPacketErrored[i]);
	}
	fprintf(fp, "Total\t%lld\t\t%lld\t\t%lld\n", numTotalPacketCaptured,
			numTotalPacketReinjected, numTotalPacketErrored);
	fprintf(fp, "More Feature is comming soon...\n");
	fclose(fp);
	return 0;
}

void create_metric_thread()
{
	UINT appCount = appmap_get_map_count();
	
	numPacketCaptured = calloc(appCount, sizeof * numPacketCaptured);
	numPacketReinjected = calloc(appCount, sizeof * numPacketReinjected);
	numPacketErrored = calloc(appCount, sizeof * numPacketErrored);
	
	metricThread = CreateThread(NULL, 0, write_metric, NULL, 0, &metricThreadId);
}

void close_metric_thread()
{
	WaitForSingleObject(metricThread, INFINITE);
	CloseHandle(metricThread);

	free(numPacketCaptured);
	free(numPacketReinjected);
}

void metric_packet_capture(UINT appId, UINT count)
{
	numTotalPacketCaptured += count;
	numPacketCaptured[appId - 1] += count;
}

void metric_packet_reinjected(UINT appId)
{
	numTotalPacketReinjected++;
	numPacketReinjected[appId - 1]++;
}

void metric_packet_errored(UINT appId)
{
	numTotalPacketErrored++;
	numPacketErrored[appId - 1]++;
}
