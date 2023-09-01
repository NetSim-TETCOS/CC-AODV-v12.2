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
#define _NO_VIRTUAL_
#pragma warning(disable:4214)

#include "main.h"
#include "EmulationLib.h"
#include "FastEmulation.h"
#include "EmulationPacket.h"
#include "NetSim_utility.h"
#include "windivert.h"
#include "ApplicationMap.h"

//#define DEFAULT_LAYER WINDIVERT_LAYER_NETWORK
#define DEFAULT_LAYER WINDIVERT_LAYER_NETWORK_FORWARD

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"windivert.lib")

HANDLE* hCaptureThread;
DWORD* captureThreadId;
HANDLE* hReinjectThread; //Only for independent application
DWORD* reinjectThreadId;
HANDLE* windivertHandle;

typedef struct stru_param
{
	UINT id;
	char* filterString;
	INT16 priority;
	ptrAPPMAP map;
}PARAM, * ptrPARAM;

ptrPARAM captureParam;

static void print_packet_info(void* handle, ptrREALPACKET packet)
{
	UINT i;
	for(i=0;i<packet->realPacketCount;i++)
	{

		write_to_pcap(handle, packet->eachRealPacketAddr[i],
					  packet->eachRealPacketLen[i], NULL);
	}
}

static ptrREALPACKET form_real_packet_str(UINT8* packet, UINT recvLen,
										  PWINDIVERT_ADDRESS addr, HANDLE windivertHandle)
{
	ptrREALPACKET r = calloc(1, sizeof * r);
	r->realPacket = calloc((size_t)recvLen + 10, sizeof(UINT8));
	memcpy(r->realPacket, packet, recvLen * sizeof(UINT8));
	r->realPacketLen = recvLen;
	r->windivertAddr = addr;
	r->windivertAddrLen = sizeof * addr;
	r->windivertHandle = windivertHandle;

	UINT i = 0;
	UINT8* p = r->realPacket;
	UINT plen = recvLen;

	UINT8* pnext = NULL;
	UINT pnextlen = 0;

	while (p)
	{
		WinDivertHelperParsePacket(p, plen,
								   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
								   &pnext, &pnextlen);
		UINT currlen = plen - pnextlen;

		r->eachRealPacketAddr[i] = p;
		r->eachRealPacketLen[i] = currlen;
		r->realPacketCount++;

		r->eachPacketwindivertAddr[i] = addr;

		addr += 1;
		i++;
		p = pnext;
		plen = pnextlen;
	}
	return r;
}

static HANDLE open_windivert_handle(char* filterString, INT16 priority)
{
	HANDLE h;
	h = WinDivertOpen(filterString, DEFAULT_LAYER, priority, 0);

	if (h == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_INVALID_PARAMETER)
		{
			fnNetSimError("error: filter syntax error\n");
			return NULL;
		}
		fnSystemError("error: failed to open the WinDivert device.\n");
		return NULL;
	}

	return h;
}

int main_Pkt_capture(UINT id, char* filterString, INT16 priority, ptrAPPMAP map)
{
	UINT maxPacketLen = MAX_PACKET_LEN * MAX_PACKET_COUNT;
	maxPacketLen = (maxPacketLen < WINDIVERT_MTU_MAX ? WINDIVERT_MTU_MAX : maxPacketLen);
	unsigned char* packet = calloc(maxPacketLen, sizeof(char));
	UINT recvLen;

	UINT addrLen = MAX_PACKET_COUNT * sizeof(WINDIVERT_ADDRESS);
	WINDIVERT_ADDRESS* addr = calloc(MAX_PACKET_COUNT, sizeof(WINDIVERT_ADDRESS));

	WaitForSingleObject(map->mutex, INFINITE);
	ReleaseMutex(map->mutex);

	fprintf(stderr, "\nCalling NetSim Emulation capture for %d application...\n", id);
	
	// Divert traffic matching the filter:
	windivertHandle[id - 1] = open_windivert_handle(filterString, priority);
	if (windivertHandle[id - 1] == NULL)
	{
		fnSystemError("Unable to open windivert handle for application %d\n", id);
		return -1;
	}

	// Main loop:
	while (map->isAppStarted)
	{
		// Read a matching packet.
		if (!WinDivertRecvEx(windivertHandle[id-1], packet,
							 maxPacketLen, &recvLen,
							 0, addr,
							&addrLen, NULL))
		{
			fnSystemError("warning: failed to read packet\n");
			continue;
		}

		ptrREALPACKET realPacket = form_real_packet_str(packet, recvLen, addr, windivertHandle[id - 1]);
		if (dispatchHandle)
			print_packet_info(dispatchHandle, realPacket);

		dispatch_to_emulation(realPacket, map);
	}
	free(packet);
	free(addr);
	return 0;
}

DWORD WINAPI captureFunction(ptrPARAM lpParam)
{
	fprintf(stderr, "Starting thread for application %d instance %d\n", lpParam->map->appId, lpParam->map->instanceId);

	main_Pkt_capture(lpParam->id, lpParam->filterString, lpParam->priority, lpParam->map);
	return 0;
}

DWORD WINAPI reinject_independent_app(ptrPARAM lpParam)
{
	ptrAPPMAP map = lpParam->map;

	WaitForSingleObject(map->mutex, INFINITE);
	ReleaseMutex(map->mutex);

	fprintf(stderr, "Starting reinject thread for application %d, instance %d\n", map->appId, map->instanceId);

	ptrPACKET p = NULL;
	while (map->isAppStarted)
	{
		if (!p)
		{
			p = get_head_packet_ptr_from_app_map(map);
			if (!p)
			{
				usleep(1);
				continue;
			}
		}

		UINT64 t = em_current_time();
		UINT64 t1 = p->reinjectTime;

		if (t >= t1)
		{
			p = get_packet_from_app_map(map);
			reinject_pkt(p);
			free_packet(p);
			p = NULL;
		}
		else
		{
			usleep(t1 - t);
		}
	}
	return 0;
}

void create_capture_thread()
{
	UINT count = appmap_get_map_count();

	hCaptureThread = calloc(count, sizeof * hCaptureThread);
	windivertHandle = calloc(count, sizeof * windivertHandle);
	captureThreadId = calloc(count, sizeof * captureThreadId);
	captureParam = calloc(count, sizeof * captureParam);
	hReinjectThread = calloc(count, sizeof * hReinjectThread);
	reinjectThreadId = calloc(count, sizeof * reinjectThreadId);

	UINT i;
	for (i = 0; i < count; i++)
	{
		ptrAPPMAP map = appmap_get_map(i + 1);
		captureParam[i].id = i + 1;
		captureParam[i].filterString = appmap_get_filter_string(i + 1);
		captureParam[i].priority = 0;
		captureParam[i].map = map;

		hCaptureThread[i] = CreateThread(NULL, 0, captureFunction, &captureParam[i], 0, &captureThreadId[i]);
		if (!hCaptureThread[i])
			fnSystemError("Unable to create capture thread for application %d.\n", i + 1);

		if (map->isNonInterferingApp)
		{
			hReinjectThread[i] = CreateThread(NULL, 0, reinject_independent_app, &captureParam[i], 0, &reinjectThreadId[i]);
			if (!hReinjectThread)
				fnSystemError("Unable to create reinject thread for application %d.\n", i + 1);

			char mutexName[50];
			sprintf(mutexName, "APP_MAP_%d", i + 1);
			map->queueMutex = CreateMutexA(NULL, false, mutexName);
		}
	}
}

void close_capture_thread()
{
	UINT count = appmap_get_map_count();
	UINT i;
	for (i = 0; i < count; i++)
	{
		close_application(appmap_get_map(i + 1));
	}
	Sleep(1000);
	for (i = 0; i < count; i++)
	{
		WinDivertShutdown(windivertHandle[i], WINDIVERT_SHUTDOWN_BOTH);
		WinDivertClose(windivertHandle[i]);
		WaitForSingleObject(hCaptureThread[i], INFINITE);

		if (hReinjectThread[i])
			WaitForSingleObject(hReinjectThread[i], INFINITE);
	}

	free(hCaptureThread);
	free(windivertHandle);
	free(captureThreadId);
	free(captureParam);
	free(hReinjectThread);
}

int reinject_pkt(ptrPACKET packet)
{	
	UINT i;
	for (i = 0; i < packet->realPacket->realPacketCount; i++)
	{
		if (packet->realPacket->isPacketErroed[i] != 0)
		{
			metric_packet_errored(packet->appId);
			continue; // packet got erroed
		}

		metric_packet_reinjected(packet->appId);

		if (reinjectHandle)
			write_to_pcap(reinjectHandle, packet->realPacket->eachRealPacketAddr[i],
						  packet->realPacket->eachRealPacketLen[i], NULL);

		if (!WinDivertSend(packet->realPacket->windivertHandle, packet->realPacket->eachRealPacketAddr[i],
						   packet->realPacket->eachRealPacketLen[i], NULL,
						   packet->realPacket->eachPacketwindivertAddr[i]))
		{
			fnSystemError("warning: failed to reinject packet\n");
		}
	}
	return 0;
}
