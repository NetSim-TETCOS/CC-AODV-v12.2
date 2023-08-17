#pragma once
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
#ifndef _NETSIM_FASTEMULATION_H_
#define _NETSIM_FASTEMULATION_H_
#ifdef  __cplusplus
extern "C" {
#endif

#include "EmulationLib.h"
#define MAX_PACKET_LEN		4096
#define MAX_PACKET_COUNT	50

	HANDLE fastEmulationSyncMutex;
	bool isContinueEmulation;
	
	typedef struct stru_queue QUEUE, * ptrQUEUE;
	typedef struct stru_emulation_packet PACKET, * ptrPACKET;
	typedef struct stru_real_packet REALPACKET, * ptrREALPACKET;
	typedef struct stru_app_map APPMAP, * ptrAPPMAP;

	typedef enum enum_event_type
	{
		EVENTTYPE_NULL,
		EVENTTYPE_CAPTURE,
		EVENTTYPE_REINJECT,

		EVENTTYPE_QUEUE,

		EVENTTYPE_APPSTART,
		EVENTTYPE_APPEND,
	}EVENTTYPE;
	static char strEVENTTYPE[][50] = { "NULL","CAPTURE","REINJECT","QUEUE","APPSTART","APPEND" };

	typedef struct stru_event_details
	{
		EVENTTYPE type;
		UINT64 time;

		void* appMap;

		struct stru_event_details* next;
	}EVENTDETAILS, * ptrEVENTDETAILS;

	//Function prototype
	//App map
	void fastemulation_init_app_map();
	void fastemulation_setup_path();
	void dispatch_to_emulation(ptrREALPACKET packet, ptrAPPMAP map);
	UINT appmap_get_map_count();
	char* appmap_get_filter_string(UINT id);
	ptrAPPMAP appmap_get_map(UINT id);
	void fastemulation_setup_non_interfering_app();
	ptrAPPMAP appmap_find(NETSIM_ID appId, UINT instanceId);
	void add_appmap_event();
	void start_application(ptrAPPMAP map);
	void close_application(ptrAPPMAP map);

	//QUEUE
	void fastemulation_init_queue();
	ptrQUEUE find_queue(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri);
	void add_packet_to_queue(ptrPACKET p, UINT64 time);
	ptrPACKET get_packet_from_queue(ptrQUEUE q);
	void create_queue_thread();

	//Packet capture
	void create_capture_thread();
	void close_capture_thread();
	int reinject_pkt(ptrPACKET packet);

	//Emulation packet
	ptrPACKET create_new_packet(ptrREALPACKET packet, UINT64 time);
	void free_packet(ptrPACKET packet);

	//Only for independent application
	void add_packet_to_app_map(ptrAPPMAP map, ptrPACKET packet);
	ptrPACKET get_packet_from_app_map(ptrAPPMAP map);
	ptrPACKET get_head_packet_ptr_from_app_map(ptrAPPMAP map);

	//Metric
	void create_metric_thread();
	void close_metric_thread();
	void metric_packet_capture(UINT appId, UINT count);
	void metric_packet_reinjected(UINT appId);
	void metric_packet_errored(UINT appId);

	//Error model
	void errormodel_check_packet_for_error(ptrREALPACKET p, ptrAPPMAP map);

	//Route file create
	int fastemulation_Route_file();

	//Event Handler
	void init_eventhandler();
	void start_event_execution();
	void add_event(ptrEVENTDETAILS e);

#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_FASTEMULATION_H_
