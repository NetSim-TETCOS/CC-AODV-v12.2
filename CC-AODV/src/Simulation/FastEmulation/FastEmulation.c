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
#include "EmulationLib.h"
#include "FastEmulation.h"
#pragma comment(lib,"NetworkStack.lib")

int pcap_logfile_status[HANDLE_COUNT];

_declspec(dllexport) int init_fast_emulation(int* pcapLogStatus)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	fprintf(stderr, "Creating emulation log file.\n");
	memcpy(pcap_logfile_status, pcapLogStatus, 4 * sizeof(int));
	create_emulation_pcap_handle(HANDLE_COUNT, pcap_logfile_status, (char**)pcapFileName, (char**)pipeName, handles);

	fprintf(stderr, "Creating application map.\n");
	fastemulation_init_app_map();

	fprintf(stderr, "Creating Queues.\n");
	fastemulation_init_queue();

	fprintf(stderr, "Creating Route file\n");
	fastemulation_Route_file();

	fprintf(stderr, "Setting up path for each application.\n");
	fastemulation_setup_path();
	fastemulation_setup_non_interfering_app();

	fastEmulationSyncMutex = CreateMutexA(NULL, true, "Fast_emulation_sync_mutex");

	create_queue_thread();
	create_capture_thread();
	create_metric_thread();

	init_eventhandler();
	add_appmap_event();
	return 0;
}

_declspec(dllexport) void start_fast_emulation()
{
	em_init_time();
	ReleaseMutex(fastEmulationSyncMutex);
	start_event_execution();
}

_declspec(dllexport) void close_fast_emulation()
{
	close_capture_thread();
	close_metric_thread();
	close_emulation_pcap_handle(HANDLE_COUNT, handles);
}
