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
#include "Satellite_PHY.h"

#pragma region EVENT_OTHER_DETAILS
typedef struct stru_eventdetails
{
	ptrSUPERFRAME sf;
	ptrCARRIER carrier;
	ptrFRAME fr;
}EVENTDETAILS, * ptrEVENTDETAILS;

static ptrEVENTDETAILS form_eventdetails(ptrSUPERFRAME sf, ptrFRAME fr, ptrCARRIER carrier)
{
	ptrEVENTDETAILS ev = calloc(1, sizeof * ev);
	ev->fr = fr;
	ev->sf = sf;
	ev->carrier = carrier;
	return ev;
}

static void clear_eventdetails(ptrEVENTDETAILS ev)
{
	free(ev);
}

static ptrSUPERFRAME get_curr_superframe()
{
	ptrEVENTDETAILS ev = pstruEventDetails->szOtherDetails;
	return ev->sf;
}

static ptrFRAME get_curr_frame()
{
	ptrEVENTDETAILS ev = pstruEventDetails->szOtherDetails;
	return ev->fr;
}

static ptrCARRIER get_curr_carrier()
{
	ptrEVENTDETAILS ev = pstruEventDetails->szOtherDetails;
	return ev->carrier;
}
#pragma endregion

#pragma region FUNCTION_PROTOTYPE
static void add_superframe_event(NETSIM_ID d, NETSIM_ID in, double time, ptrSUPERFRAME sf);
static void frame_send_packet(NETSIM_ID d, NETSIM_ID in,
							  ptrSUPERFRAME sf, ptrFRAME fr, ptrCARRIER cr);
#pragma endregion

#pragma region UTILITY
ptrSUPERFRAME satellite_get_return_superframe(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_MAC mac = SATELLITE_MAC_GET(d, in);
	return mac->returnLinkSuperFrame;
}

ptrSUPERFRAME satellite_get_forward_superframe(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_MAC mac = SATELLITE_MAC_GET(d, in);
	return mac->forwardFLinkSuperFrame;
}
#pragma endregion

#pragma region SUPERFRAME_INIT
ptrSUPERFRAME superframe_alloc(LINKTYPE linkType,
							   NETSIM_ID d, NETSIM_ID in)
{
	ptrSUPERFRAME sf = calloc(1, sizeof * sf);
	sf->carrierConfig = calloc(1, sizeof * sf->carrierConfig);
	sf->slotConfig = calloc(1, sizeof * sf->slotConfig);
	sf->frameConfig = calloc(1, sizeof * sf->frameConfig);
	sf->linkType = linkType;

	ptrSATELLITE_MAC mac = SATELLITE_MAC_GET(d, in);
	ptrSATELLITE_PHY phy = SATELLITE_PHY_GET(d, in);

	if (linkType == LINKTYPE_FORWARD)
	{
		mac->forwardFLinkSuperFrame = sf;
		phy->forwardLinkSuperFrame = sf;
	}
	else if (linkType == LINKTYPE_RETURN)
	{
		mac->returnLinkSuperFrame = sf;
		phy->returnLinkSuperFrame = sf;
	}

	return sf;
}

static void form_frames(ptrSUPERFRAME sf, ptrCARRIER carrier)
{
	UINT i;
	
	carrier->frameCount = sf->frameConfig->framesPerSuperframe;
	carrier->frames = calloc(carrier->frameCount, sizeof * carrier->frames);

	double startTime = 0;
	for (i = 0; i < carrier->frameCount; i++)
	{
		ptrFRAME fr = calloc(1, sizeof * fr);
		carrier->frames[i] = fr;

		fr->relativeFrameStartTime = startTime;

		fr->relativeFrameEndTime = sf->frameConfig->frameLength_us + startTime;
		fr->bandwidth_Hz = carrier->bandwidth_Hz;
		fr->carrierId = carrier->carrierId;
		fr->frameId = i + 1;
		fr->isRandomAccess = false;

		startTime = fr->relativeFrameEndTime;
	}
}

static UINT calculate_bitsPerSlot(ptrSUPERFRAME sf)
{
	return (UINT)floor((double)sf->slotConfig->symbolPerSlot *
		(double)sf->slotConfig->modulationBits *
					   sf->carrierConfig->codingRate);
}

static UINT getModulationBits(PHY_MODULATION modulation)
{
	switch (modulation)
	{
		case Modulation_QPSK:
		{
			return 2;
			break;
		}
		case Modulation_8PSK:
		{
			return 3;
			break;
		}
		case Modulation_16APSK:
		case Modulation_16_QAM:
		{
			return 4;
			break;
		}
		case Modulation_32APSK:
		{
			return 5;
			break;
		}
		default:
		{
			fnNetSimError("Unsupported modulation %s  for satellite network\n",
						  strPHY_MODULATION[modulation]);
			return 0;
			break;
		}
	}
}

static void form_carrier(ptrSUPERFRAME sf)
{
	UINT i;
	
	sf->carrierCount = (UINT)floor(sf->bandwidth_Hz / sf->carrierConfig->allocatedBandwidth_Hz);
	sf->carriers = calloc(sf->carrierCount, sizeof * sf->carriers);
	print_satellite_log("Carrier count = %d\n", sf->carrierCount);
	satellite_log_add_tab();

	for (i = 0; i < sf->carrierCount; i++)
	{
		print_satellite_log("Forming carrier %d\n", i + 1);

		sf->carriers[i] = calloc(1, sizeof * sf->carriers[i]);
		ptrCARRIER cr = sf->carriers[i];
		cr->bandwidth_Hz = sf->carrierConfig->allocatedBandwidth_Hz;
		cr->carrierId = i + 1;
		cr->centerFrequency_Hz = sf->baseFreqnecy_Hz + cr->bandwidth_Hz * (i + 0.5);

		print_satellite_log("Carrier Id = %d\n", i + 1);
		print_satellite_log("carrier allocated bandwidth = %lf Hz\n", cr->bandwidth_Hz);
		print_satellite_log("Central frequency = %lf Hz\n", cr->centerFrequency_Hz);
		print_satellite_log("\n");
	}
	satellite_log_remove_tab();
	print_satellite_log("\n");
}

static void configure_superframe(ptrSUPERFRAME sf)
{
	sf->bandwidth_Hz = sf->frameConfig->frameBandwidth_Hz;
	sf->centralFrequency_Hz = sf->baseFreqnecy_Hz + sf->bandwidth_Hz / 2.0;
	sf->slotConfig->symbolRate = (UINT)sf->carrierConfig->effectiveBandwidth_Hz;
	sf->slotConfig->modulationBits = getModulationBits(sf->carrierConfig->modulation);

	UINT slots = sf->frameConfig->slotCountInFrame + sf->frameConfig->plHdrSize_inSlots;
	UINT dataSymbols = slots * sf->slotConfig->symbolPerSlot;
	UINT pilotSlot = (UINT)floor((double)slots / sf->frameConfig->pilotBlockInterval_inSlots);
	UINT pilotSymbol = pilotSlot * sf->frameConfig->pilotBlockSize_inSymbols;
	UINT totalSymbol = pilotSymbol + dataSymbols;

	sf->frameConfig->frameLength_us = (((double)totalSymbol) / sf->slotConfig->symbolRate) * SECOND;
	sf->frameConfig->pilotBlockLength_us = (((double)sf->frameConfig->pilotBlockSize_inSymbols) / sf->slotConfig->symbolRate) * SECOND;
	sf->slotConfig->slotLength_us = (((double)sf->slotConfig->symbolPerSlot) / sf->slotConfig->symbolRate) * SECOND;
	sf->superFrameDuration_us = sf->frameConfig->frameLength_us * sf->frameConfig->framesPerSuperframe;
	sf->slotConfig->bitsPerSlot = calculate_bitsPerSlot(sf);
	sf->frameConfig->bitsPerFrame = sf->slotConfig->bitsPerSlot * sf->frameConfig->slotCountInFrame;

	print_satellite_log("Frame length = %lf us\n", sf->frameConfig->frameLength_us);
	print_satellite_log("Pilot block length = %lf us\n", sf->frameConfig->pilotBlockLength_us);
	print_satellite_log("Slot length = %lf us\n", sf->slotConfig->slotLength_us);
	print_satellite_log("Superframe Duration = %lf us\n", sf->superFrameDuration_us);
	print_satellite_log("Bits per slot = %d\n", sf->slotConfig->bitsPerSlot);
	print_satellite_log("Bits per frame = %d\n", sf->frameConfig->bitsPerFrame);
	print_satellite_log("Superframe duration = %0.3lf us\n", sf->superFrameDuration_us);
	print_satellite_log("Superframe bandwidth = %lf Hz\n", sf->bandwidth_Hz);
	print_satellite_log("Central frquency = %lf Hz\n", sf->centralFrequency_Hz);
}

static void form_superframe(ptrSUPERFRAME sf)
{
	UINT i;

	configure_superframe(sf);
	form_carrier(sf);
	
	for (i = 0; i < sf->carrierCount; i++)
		form_frames(sf, sf->carriers[i]);
}

static void set_max_payload_size(NETSIM_ID d, NETSIM_ID in,
								 UINT bits)
{
	UINT bytes = bits / 8;
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;
	NETSIM_ID i;
	for (i = 0; i < link->puniDevList.pstrup2MP.nConnectedDeviceCount - 1; i++)
	{
		NETSIM_ID c = link->puniDevList.pstrup2MP.anDevIds[i];
		NETSIM_ID cin = link->puniDevList.pstrup2MP.anDevInterfaceIds[i];
		DEVICE_MACLAYER(c, cin)->dFragmentSize = (double)bytes;
	}
}

static void set_max_unit_size(ptrSUPERFRAME sf, UINT maxBitsCount)
{
	double maxBytes = floor(maxBitsCount / 8.0);
	UINT i;
	for (i = 0; i < sf->bufferCount; i++)
		satellite_buffer_setMaxUnitSizeInBytes(sf->buffers[i], maxBytes);
}

void superframe_init(NETSIM_ID d,NETSIM_ID in)
{
	ptrSATELLITE_MAC mac = SATELLITE_MAC_GET(d, in);
	ptrSUPERFRAME rsf = mac->returnLinkSuperFrame;
	ptrSUPERFRAME fsf = mac->forwardFLinkSuperFrame;

	print_satellite_log("\n\nForming return superframe for satellite %d-%d.\n", d, in);
	form_superframe(rsf);

	print_satellite_log("\n\nForming forward superframe for satellite %d-%d.\n", d, in);
	form_superframe(fsf);

	set_max_payload_size(d, in, min(rsf->frameConfig->bitsPerFrame, fsf->frameConfig->bitsPerFrame));
	set_max_unit_size(rsf, min(rsf->frameConfig->bitsPerFrame, fsf->frameConfig->bitsPerFrame));
	set_max_unit_size(fsf, min(rsf->frameConfig->bitsPerFrame, fsf->frameConfig->bitsPerFrame));
	add_superframe_event(d, in, 0, rsf);
	add_superframe_event(d, in, 0, fsf);
}
#pragma endregion

#pragma region SUPERFRAME_RESET
static void reset_carrier(ptrSUPERFRAME sf, ptrCARRIER cr)
{
	UINT i;
	for (i = 0; i < cr->frameCount; i++)
	{
		cr->frames[i]->frameStartTime = 0;
		cr->frames[i]->frameEndTime = 0;
	}
}

static void reset_superframe(ptrSUPERFRAME sf)
{
	UINT i;
	for (i = 0; i < sf->carrierCount; i++)
		reset_carrier(sf, sf->carriers[i]);

	sf->startTime_us = 0;
	sf->endTime_us = 0;
}
#pragma endregion

#pragma region FRAME_EVENT
static void add_frame_start_event(NETSIM_ID d, NETSIM_ID in,
								  ptrSUPERFRAME sf, ptrFRAME fr, ptrCARRIER cr)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = fr->frameStartTime;
	pevent.nDeviceId = d;
	pevent.nDeviceType = DEVICE_TYPE(d);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = in;
	pevent.nProtocolId = MAC_PROTOCOL_SATELLITE;
	pevent.nSubEventType = SUBEVENT_FRAME_START;
	pevent.szOtherDetails = form_eventdetails(sf, fr, cr);
	fnpAddEvent(&pevent);
}

void satellite_frame_start()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	double time = pstruEventDetails->dEventTime;
	ptrSATELLITE_MAC mac = SATELLITE_MAC_GET(d, in);
	ptrFRAME fr = get_curr_frame();
	ptrCARRIER cr = get_curr_carrier();
	ptrSUPERFRAME sf = get_curr_superframe();
	clear_eventdetails(pstruEventDetails->szOtherDetails);

	print_satellite_log("Satellite %d-%d, Time %0.3lf us. Starting Frame\n",
						d, in, time);
	satellite_log_add_tab();
	print_satellite_log("Frame start time = %0.3lf us\n", fr->frameStartTime);
	print_satellite_log("Frame end time = %0.3lf us\n", fr->frameEndTime);
	
	satellite_allocate_slot(d, in, sf, fr);
	frame_send_packet(d, in, sf, fr, cr);
	satellite_log_remove_tab();
}
#pragma endregion

#pragma region SUPERFRAME_EVENT
static void add_superframe_event(NETSIM_ID d, NETSIM_ID in, double time, ptrSUPERFRAME sf)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = d;
	pevent.nDeviceType = DEVICE_TYPE(d);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = in;
	pevent.nProtocolId = MAC_PROTOCOL_SATELLITE;
	pevent.nSubEventType = SUBEVENT_SUPERFRAME_START;
	pevent.szOtherDetails = form_eventdetails(sf, NULL, NULL);
	fnpAddEvent(&pevent);
}

static void start_superframe(ptrSATELLITE_MAC mac, ptrSUPERFRAME sf, double time)
{
	sf->startTime_us = time;
	sf->endTime_us = sf->superFrameDuration_us + time;
	sf->superFrameId++;
	print_satellite_log("Superframe Id = %d\n", sf->superFrameId);
	print_satellite_log("Superframe start time = %0.3lf us\n", sf->startTime_us);
	print_satellite_log("Superframe end time = %0.3lf us\n", sf->endTime_us);
	print_satellite_log("Link Type = %s\n", strLINKTYPE[sf->linkType]);

	UINT i, j;
	for (i = 0; i < sf->carrierCount; i++)
	{
		ptrCARRIER carrier = sf->carriers[i];
		for (j = 0; j < carrier->frameCount; j++)
		{
			ptrFRAME fr = carrier->frames[j];
			fr->frameStartTime = fr->relativeFrameStartTime + time;
			fr->frameEndTime = fr->relativeFrameEndTime + time;
			print_satellite_log("Frame %d start time = %0.3lf us\n", i + 1, fr->frameStartTime);
			print_satellite_log("Frame %d end time = %0.3lf us\n", i + 1, fr->frameEndTime);
			print_satellite_log("Adding frame %d start event\n", i + 1);
			add_frame_start_event(mac->satelliteId, mac->satelliteIf, sf, fr, carrier);
		}
	}
}

void satellite_superframe_start()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrSATELLITE_MAC mac = SATELLITE_MAC_GET(d, in);
	ptrSUPERFRAME sf = get_curr_superframe();
	clear_eventdetails(pstruEventDetails->szOtherDetails);

	if (sf->bufferCount == 0)
		satellite_form_bufferList(d, in, sf);

	double time = pstruEventDetails->dEventTime;
	
	print_satellite_log("Satellite %d-%d, Time %0.3lf us\n",
						d, in, time);
	satellite_log_add_tab();
	print_satellite_log("Resetting previous superframe\n");
	reset_superframe(sf);
	print_satellite_log("Starting new superframe\n");
	start_superframe(mac, sf, time);

	//Add next superframe start event
	time += sf->superFrameDuration_us;
	add_superframe_event(d, in, time, sf);
	print_satellite_log("Adding next superframe start event at %0.3lf us\n", time);
	satellite_log_remove_tab();
}
#pragma endregion

#pragma region FRAME_SEND
static void send_packet_to_phy(NetSim_PACKET* packet, 
							   NETSIM_ID d, NETSIM_ID in)
{
	packet->pstruMacData->dPacketSize = packet->pstruMacData->dPayload + packet->pstruMacData->dOverhead;
	packet->pstruPhyData->dPayload = packet->pstruMacData->dPacketSize;
	packet->pstruPhyData->dPacketSize = packet->pstruPhyData->dOverhead +
		packet->pstruPhyData->dPayload;

	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = packet->pstruMacData->dEndTime;
	pevent.dPacketSize = packet->pstruMacData->dPacketSize;

	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}
	
	pevent.nDeviceId = packet->nTransmitterId;
	pevent.nDeviceType = DEVICE_TYPE(packet->nTransmitterId);
	pevent.nEventType = PHYSICAL_OUT_EVENT;
	pevent.nInterfaceId = fn_NetSim_Stack_GetConnectedInterface(d, in, packet->nTransmitterId);;
	pevent.nPacketId = packet->nPacketId;
	pevent.nProtocolId = MAC_PROTOCOL_SATELLITE;
	pevent.pPacket = packet;
	fnpAddEvent(&pevent);
}

static void update_time(NetSim_PACKET* packet, double* startTime,
						UINT bitsPerFrame, double frameDuration)
{
	UINT bitsCount = (UINT)packet->pstruMacData->dPayload * 8;
	double duration = ((bitsCount * 1.0) / bitsPerFrame) * frameDuration;

	packet->pstruMacData->dEndTime = *startTime;
	packet->pstruMacData->dStartTime = *startTime;

	packet->pstruPhyData->dArrivalTime = *startTime;
	packet->pstruPhyData->dStartTime = *startTime + duration;

	*startTime += duration;
}

static void frame_send_packet(NETSIM_ID d, NETSIM_ID in,
							  ptrSUPERFRAME sf, ptrFRAME fr, ptrCARRIER cr)
{
	double st = fr->frameStartTime;
	double frDur = fr->frameEndTime - fr->frameStartTime;
	NetSim_PACKET* packet;
	while (fr->head)
	{
		packet = fr->head;
		fr->head = packet->pstruNextPacket;
		if (fr->head == NULL) fr->tail = NULL;
		packet->pstruNextPacket = NULL;

		update_time(packet, &st, sf->frameConfig->bitsPerFrame, frDur);

		send_packet_to_phy(packet, d, in);

	}
}
#pragma endregion
