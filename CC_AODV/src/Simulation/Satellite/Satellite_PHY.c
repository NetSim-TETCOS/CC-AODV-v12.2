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
#include "Satellite_PHY.h"
#include "Satellite_MAC.h"
#include "Satellite_HDR.h"
#include "Satellite_PropagationModel.h"

#pragma region FUNCTION_PROTOTYPE
#pragma endregion

#pragma region SATELLITE_PHY_INIT
ptrSATELLITE_PHY satellite_phy_alloc(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_PHY phy = calloc(1, sizeof * phy);
	phy->d = d;
	phy->in = in;
	SATELLITE_PHY_SET(d, in, phy);
	return phy;
}

ptrSATELLITE_GW_PHY satellite_gw_phy_alloc(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_GW_PHY phy = calloc(1, sizeof * phy);
	phy->d = d;
	phy->in = in;
	SATELLITE_GWPHY_SET(d, in, phy);
	return phy;
}

ptrSATELLITE_UT_PHY satellite_ut_phy_alloc(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_UT_PHY phy = calloc(1, sizeof * phy);
	phy->d = d;
	phy->in = in;
	SATELLITE_UTPHY_SET(d, in, phy);
	return phy;
}

void satellite_ut_phy_init(NETSIM_ID d, NETSIM_ID in)
{
	satellite_propgation_ut_init(d, in);
	satellite_propagation_ut_calculate_rxpower(d, in, 0);
}

void satellite_gw_phy_init(NETSIM_ID d, NETSIM_ID in)
{
	satellite_propgation_gw_init(d, in);
	satellite_propagation_gw_calculate_rxpower(d, in, 0);
}
#pragma endregion

#pragma region HELPER_FUNCTION
#define SPEED_OF_LIGHT 299792458.0 // m/s
static double calculate_propagation_delay(NETSIM_ID d, NETSIM_ID r)
{
	assert(d);
	assert(r);
	double distance = DEVICE_DISTANCE(d, r);
	return (distance / SPEED_OF_LIGHT) * SECOND;
}

static void satellite_send_packet(NetSim_PACKET* packet,
								  NETSIM_ID d, NETSIM_ID in,
								  NETSIM_ID r, NETSIM_ID rin)
{
	double propagationDelay = calculate_propagation_delay(d, r);
	packet->pstruPhyData->dEndTime = packet->pstruPhyData->dStartTime + propagationDelay;
	packet->nTransmitterId = d;
	packet->nReceiverId = r;

	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = packet->pstruPhyData->dEndTime;
	pevent.dPacketSize = packet->pstruPhyData->dPacketSize;
	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}
	pevent.nDeviceId = r;
	pevent.nDeviceType = DEVICE_TYPE(r);
	pevent.nEventType = PHYSICAL_IN_EVENT;
	pevent.nInterfaceId = rin;
	pevent.nPacketId = packet->nPacketId;
	pevent.nProtocolId = MAC_PROTOCOL_SATELLITE;
	pevent.pPacket = packet;
	fnpAddEvent(&pevent);
}

static void write_trace(NetSim_PACKET* packet)
{
	fn_NetSim_WritePacketTrace(packet);
	fn_NetSim_Metrics_Add(packet);
}

static void send_to_mac(NETSIM_ID d, NETSIM_ID in, NetSim_PACKET* packet, double time)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.dPacketSize = packet->pstruMacData->dPacketSize;
	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}
	pevent.nDeviceId = d;
	pevent.nInterfaceId = in;
	pevent.nDeviceType = DEVICE_TYPE(d);
	pevent.nEventType = MAC_IN_EVENT;
	pevent.nPacketId = packet->nPacketId;
	pevent.nProtocolId = MAC_PROTOCOL_SATELLITE;
	pevent.pPacket = packet;
	fnpAddEvent(&pevent);
}
#pragma endregion

#pragma region UTPHY_PACKET_PROCESSING
static void satellite_utphy_handle_phy_out()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrSATELLITE_HDR hdr = packet->pstruMacData->Packet_MACProtocol;
	satellite_send_packet(packet, d, in, hdr->satelliteId, hdr->satelliteIf);
}

static void satellite_ut_handle_phyIn()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	double time = pstruEventDetails->dEventTime;
	NETSIM_ID t = packet->nTransmitterId;
	NETSIM_ID ti = fn_NetSim_Stack_GetConnectedInterface(d, in, t);

	if (packet->nPacketStatus != PacketStatus_NoError)
	{
		write_trace(packet);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	satellite_check_for_packet_error(t, ti, d, in, packet);
	if (packet->nPacketStatus != PacketStatus_NoError)
	{
		write_trace(packet);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	write_trace(packet);
	send_to_mac(d, in, packet, time);
}
#pragma endregion

#pragma region GWPHY_PACKET_PROCESSING
static void satellite_gwphy_handle_phy_out()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrSATELLITE_HDR hdr = packet->pstruMacData->Packet_MACProtocol;
	satellite_send_packet(packet, d, in, hdr->satelliteId, hdr->satelliteIf);
}

static void satellite_gw_handle_phyIn()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	double time = pstruEventDetails->dEventTime;
	NETSIM_ID t = packet->nTransmitterId;
	NETSIM_ID ti = fn_NetSim_Stack_GetConnectedInterface(d, in, t);

	if (packet->nPacketStatus != PacketStatus_NoError)
	{
		write_trace(packet);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	satellite_check_for_packet_error(t, ti, d, in, packet);
	if (packet->nPacketStatus != PacketStatus_NoError)
	{
		write_trace(packet);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	write_trace(packet);
	send_to_mac(d, in, packet, time);
}
#pragma endregion

#pragma region SATELLITE_PACKET_PROCESSING
static void forward_to_phyout()
{
	NetSim_EVENTDETAILS pevent;
	memcpy(&pevent, pstruEventDetails, sizeof pevent);
	pevent.nEventType = PHYSICAL_OUT_EVENT;
	fnpAddEvent(&pevent);
}

static void satellite_handle_PhyIn()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID t = packet->nTransmitterId;
	NETSIM_ID ti = fn_NetSim_Stack_GetConnectedInterface(d, in, t);

	if (packet->nPacketStatus != PacketStatus_NoError)
	{
		write_trace(packet);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	satellite_check_for_packet_error(t, ti, d, in, packet);
	if (packet->nPacketStatus != PacketStatus_NoError)
	{
		write_trace(packet);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	write_trace(packet);
	forward_to_phyout(); //Bend Pipe. So no Mac layer forwarding
}

static void satellite_forward_packet(NETSIM_ID d, NETSIM_ID in,
									 NetSim_PACKET* packet, double time,
									 NETSIM_ID r, NETSIM_ID rin)
{
	if (!r || !rin)
	{
		fnNetSimError("Receiver Id is 0 for packet %d in device %d:%d\n"
			"Make sure static route has been configured correctly\n",
			packet->nPacketId, d, in);
	}
	double txTime = packet->pstruPhyData->dStartTime - packet->pstruPhyData->dArrivalTime;
	double propDelay = calculate_propagation_delay(d, r);

	packet->pstruPhyData->dArrivalTime = time;
	packet->pstruPhyData->dStartTime = time + txTime;
	packet->pstruPhyData->dEndTime = time + txTime + propDelay;
	packet->nTransmitterId = d;
	packet->nReceiverId = r;

	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);

	pevent.dEventTime = packet->pstruPhyData->dEndTime;
	pevent.dPacketSize = packet->pstruPhyData->dPacketSize;
	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}

	pevent.nDeviceId = r;
	pevent.nDeviceType = DEVICE_TYPE(r);
	pevent.nEventType = PHYSICAL_IN_EVENT;
	pevent.nInterfaceId = rin;
	pevent.nPacketId = packet->nPacketId;
	pevent.nProtocolId = MAC_PROTOCOL_SATELLITE;
	pevent.pPacket = packet;
	fnpAddEvent(&pevent);
}

static void satellite_handle_PhyOut()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	double time = pstruEventDetails->dEventTime;

	ptrSATELLITE_HDR hdr = SATELLITE_HDR_GET(packet);
	if (hdr->linkType == LINKTYPE_FORWARD)
		satellite_forward_packet(d, in, packet, time, hdr->utId, hdr->utIf);
	else if (hdr->linkType == LINKTYPE_RETURN)
		satellite_forward_packet(d, in, packet, time, hdr->gwId, hdr->gwIf);
}
#pragma endregion

#pragma region PACKET_PROCESSING
void satellite_handle_phy_out()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	SATELLITE_DEVICETYPE type = SATELLITE_DEVICETYPE_GET(d, in);
	switch (type)
	{
		case SATELLITE_DEVICETYPE_USER_TERMINAL:
			satellite_utphy_handle_phy_out();
			break;
		case SATELLITE_DEVICETYPE_SATELLITE_GATEWAY:
			satellite_gwphy_handle_phy_out();
			break;
		case SATELLITE_DEVICETYPE_SATELLITE:
			satellite_handle_PhyOut();
			break;
		default:
			fnNetSimError("Unknown device type %s for device %d:%d is passed to function %s",
						  strSATELLITE_DEVICETYPE[type], d, in, __FUNCTION__);
			break;
	}
}

void satellite_handle_phy_in()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	SATELLITE_DEVICETYPE type = SATELLITE_DEVICETYPE_GET(d, in);
	switch (type)
	{
		case SATELLITE_DEVICETYPE_SATELLITE:
			satellite_handle_PhyIn();
			break;
		case SATELLITE_DEVICETYPE_USER_TERMINAL:
			satellite_ut_handle_phyIn();
			break;
		case SATELLITE_DEVICETYPE_SATELLITE_GATEWAY:
			satellite_gw_handle_phyIn();
			break;
		default:
			fnNetSimError("Unknwon device type %s for %d:%d in function %s\n",
						  strSATELLITE_DEVICETYPE[type], d, in, __FUNCTION__);
			break;
	}
}
#pragma endregion

