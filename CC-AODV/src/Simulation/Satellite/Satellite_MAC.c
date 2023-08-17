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
#include "Satellite_Frame.h"
#include "Satellite_HDR.h"

#pragma region FUNCTION_PROTOTYPE
#pragma endregion

#pragma region SATELLITE_UT_MAC_INIT
ptrSATELLITE_UT_MAC satellite_ut_mac_alloc(ptrSATELLITE_PROTODATA pd)
{
	ptrSATELLITE_UT_MAC mac;
	if (SATELLITE_LAYER_DATA_IsInitialized(pd->deviceId, pd->interfaceId, pd->deviceType, SATELLITE_LAYER_MAC))
	{
		mac = SATELLITE_UTMAC_GET(pd->deviceId, pd->interfaceId);
	}
	else
	{
		mac = calloc(1, sizeof * mac);
		mac->utId = pd->deviceId;
		mac->utIf = pd->interfaceId;
		SATELLITE_UTMAC_SET(pd->deviceId, pd->interfaceId, mac);
	}
	return mac;
}

static void satellite_ut_set_satellite(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_UT_MAC mac = SATELLITE_UTMAC_GET(d, in);
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;

	NETSIM_ID i;
	NETSIM_ID s;
	NETSIM_ID sin;

	s = link->puniDevList.pstrup2MP.nCenterDeviceId;
	sin = link->puniDevList.pstrup2MP.nCenterInterfaceId;

	SATELLITE_DEVICETYPE type = SATELLITE_DEVICETYPE_GET(s, sin);
	if (type == SATELLITE_DEVICETYPE_SATELLITE)
	{
		mac->satelliteId = s;
		mac->satelliteName = DEVICE_NAME(s);
		mac->satelliteIf = sin;
		return;
	}

	for (i = 0; i < link->puniDevList.pstrup2MP.nConnectedDeviceCount; i++)
	{
		s = link->puniDevList.pstrup2MP.anDevIds[i];
		sin = link->puniDevList.pstrup2MP.anDevInterfaceIds[i];

		type = SATELLITE_DEVICETYPE_GET(s, sin);
		if (type == SATELLITE_DEVICETYPE_SATELLITE)
		{
			mac->satelliteId = s;
			mac->satelliteName = DEVICE_NAME(s);
			mac->satelliteIf = sin;
			return;
		}
	}

	fnNetSimError("No satellite is connected to UT %d:%d\n", d, in);
}

void satellite_UT_MAC_init(NETSIM_ID utId, NETSIM_ID utIf)
{
	ptrSATELLITE_UT_MAC mac = SATELLITE_UTMAC_GET(utId, utIf);
	satellite_ut_set_gateway(mac);
	satellite_ut_set_satellite(utId, utIf);

	ptrSUPERFRAME rsf = satellite_get_return_superframe(mac->satelliteId, mac->satelliteIf);

	mac->buffer = satellite_buffer_init(utId, utIf,
										mac->gatewayId, mac->gatewayIf, mac->bufferSize_bits / 8,
										rsf->frameConfig->bitsPerFrame / 8);

	satellite_assoc_ut(mac->gatewayId, mac->gatewayIf,
					   utId, utIf);
}
#pragma endregion

#pragma region SATELLITE_GW_MAC_INIT
ptrSATELLITE_GW_MAC satellite_gw_mac_alloc(ptrSATELLITE_PROTODATA pd)
{
	ptrSATELLITE_GW_MAC mac;
	if (SATELLITE_LAYER_DATA_IsInitialized(pd->deviceId, pd->interfaceId, pd->deviceType, SATELLITE_LAYER_MAC))
	{
		mac = SATELLITE_GWMAC_GET(pd->deviceId, pd->interfaceId);
	}
	else
	{
		mac = calloc(1, sizeof * mac);
		mac->gwId = pd->deviceId;
		mac->gwIf = pd->interfaceId;
		SATELLITE_GWMAC_SET(pd->deviceId, pd->interfaceId, mac);
	}
	return mac;
}

static void satellite_gw_set_satellite(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_GW_MAC mac = SATELLITE_GWMAC_GET(d, in);
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;

	NETSIM_ID i;
	NETSIM_ID s;
	NETSIM_ID sin;

	s = link->puniDevList.pstrup2MP.nCenterDeviceId;
	sin = link->puniDevList.pstrup2MP.nCenterInterfaceId;

	SATELLITE_DEVICETYPE type = SATELLITE_DEVICETYPE_GET(s, sin);
	if (type == SATELLITE_DEVICETYPE_SATELLITE)
	{
		mac->satelliteId = s;
		mac->satelliteName = DEVICE_NAME(s);
		mac->satelliteIf = sin;
		return;
	}

	for (i = 0; i < link->puniDevList.pstrup2MP.nConnectedDeviceCount; i++)
	{
		s = link->puniDevList.pstrup2MP.anDevIds[i];
		sin = link->puniDevList.pstrup2MP.anDevInterfaceIds[i];

		type = SATELLITE_DEVICETYPE_GET(s, sin);
		if (type == SATELLITE_DEVICETYPE_SATELLITE)
		{
			mac->satelliteId = s;
			mac->satelliteName = DEVICE_NAME(s);
			mac->satelliteIf = sin;
			return;
		}
	}

	fnNetSimError("No satellite is connected to GW %d:%d\n", d, in);
}

void satellite_GW_MAC_init(NETSIM_ID gwId, NETSIM_ID gwIf)
{
	satellite_gw_set_satellite(gwId, gwIf);
}
#pragma endregion

#pragma region SATELLITE_MAC_INIT
ptrSATELLITE_MAC satellite_mac_alloc(ptrSATELLITE_PROTODATA pd)
{
	ptrSATELLITE_MAC mac;
	if (SATELLITE_LAYER_DATA_IsInitialized(pd->deviceId, pd->interfaceId, pd->deviceType, SATELLITE_LAYER_MAC))
	{
		mac = SATELLITE_MAC_GET(pd->deviceId, pd->interfaceId);
	}
	else
	{
		mac = calloc(1, sizeof * mac);
		mac->satelliteId = pd->deviceId;
		mac->satelliteIf = pd->interfaceId;
		SATELLITE_MAC_SET(pd->deviceId, pd->interfaceId, mac);
	}
	return mac;
}

void satellite_mac_init(NETSIM_ID d, NETSIM_ID in)
{
	superframe_init(d, in);
}
#pragma endregion

#pragma region SATELLITE_UT_ASSOCIATION
ptrSATELLITE_UTASSOCINFO satellite_utassocinfo_find(NETSIM_ID gwId, NETSIM_ID gwIf,
													NETSIM_ID utId, NETSIM_ID utIf)
{
	ptrSATELLITE_GW_MAC mac = SATELLITE_GWMAC_GET(gwId, gwIf);
	if (mac->nAssocUTCount == 0) return NULL;

	UINT i;
	for (i = 0; i < mac->nAssocUTCount; i++)
	{
		if (mac->utAssocInfo[i]->utId == utId &&
			mac->utAssocInfo[i]->utIf == utIf)
			return mac->utAssocInfo[i];
	}
	return NULL;
}

void satellite_assoc_ut(NETSIM_ID gwId, NETSIM_ID gwIf,
						NETSIM_ID utId, NETSIM_ID utIf)
{
	ptrSATELLITE_UTASSOCINFO info = satellite_utassocinfo_find(gwId, gwIf, utId, utIf);
	if (info) return; //UT is already associated

	ptrSATELLITE_GW_MAC mac = SATELLITE_GWMAC_GET(gwId, gwIf);
	if (mac->satelliteId == 0) satellite_gw_set_satellite(gwId, gwIf);
	ptrSUPERFRAME fsf = satellite_get_forward_superframe(mac->satelliteId, mac->satelliteIf);

	info = calloc(1, sizeof * info);
	info->utId = utId;
	info->utIf = utIf;
	info->buffer = satellite_buffer_init(utId, utIf, gwId, gwIf, mac->bufferSize_bits / 8, fsf->frameConfig->bitsPerFrame / 8);

	if (mac->nAssocUTCount)
		mac->utAssocInfo = realloc(mac->utAssocInfo, ((size_t)mac->nAssocUTCount + 1) * (sizeof * mac->utAssocInfo));
	else
		mac->utAssocInfo = calloc(1, sizeof * mac->utAssocInfo);

	mac->utAssocInfo[mac->nAssocUTCount] = info;
	mac->nAssocUTCount++;

	print_satellite_log("UT %d-%d is associated with gateway %d-%d\n",
						utId, utIf, gwId, gwIf);
}

#pragma endregion

#pragma region SATELLITE_UTMAC_PACKET_PROCESSING
static void satellite_utmac_hadle_mac_out()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_BUFFER* buffer = DEVICE_ACCESSBUFFER(d, in);
	ptrSATELLITE_UT_MAC mac = SATELLITE_UTMAC_GET(d, in);

	while (fn_NetSim_GetBufferStatus(buffer))
	{
		NetSim_PACKET* packet;
		packet = fn_NetSim_Packet_GetPacketFromBuffer(buffer, 1);
		
		packet->pstruMacData->dArrivalTime = pstruEventDetails->dEventTime;
		packet->pstruMacData->dPayload = packet->pstruNetworkData->dPacketSize;
		packet->pstruMacData->nMACProtocol = MAC_PROTOCOL_SATELLITE;

		satellite_hdr_init(d, in, packet);
		satellite_buffer_add_packet(mac->buffer, packet);
	}
}
#pragma endregion

#pragma region SATELLITE_GWMAC_PACKET_PROCESSING
static void satellite_gwmac_handle_mac_out()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_BUFFER* buffer = DEVICE_ACCESSBUFFER(d, in);

	while (fn_NetSim_GetBufferStatus(buffer))
	{
		NetSim_PACKET* packet;
		packet = fn_NetSim_Packet_GetPacketFromBuffer(buffer, 1);

		NETSIM_IPAddress nextHop = packet->pstruNetworkData->szNextHopIp;
		NETSIM_ID utId;
		NETSIM_ID utIf;
		utId = fn_NetSim_Stack_GetDeviceId_asIP(nextHop, &utIf);

		ptrSATELLITE_UTASSOCINFO info = satellite_utassocinfo_find(d, in, utId, utIf);

		packet->pstruMacData->dArrivalTime = pstruEventDetails->dEventTime;
		packet->pstruMacData->dPayload = packet->pstruNetworkData->dPacketSize;
		packet->pstruMacData->nMACProtocol = MAC_PROTOCOL_SATELLITE;

		satellite_hdr_init(d, in, packet);
		satellite_buffer_add_packet(info->buffer, packet);

	}
}
#pragma endregion

#pragma region SATELLITE_MAC_PACKET_PROCESSING
void satellite_handle_mac_out()
{
	ptrSATELLITE_PROTODATA pd = SATELLITE_PROTOCOLDATA_CURRENT();
	switch (pd->deviceType)
	{
		case SATELLITE_DEVICETYPE_USER_TERMINAL:
			satellite_utmac_hadle_mac_out();
			break;
		case SATELLITE_DEVICETYPE_SATELLITE_GATEWAY:
			satellite_gwmac_handle_mac_out();
			break;
		default:
			fnNetSimError("Unknwon device type in %s\n", __FUNCTION__);
			break;
	}
}

static void forward_to_network(NETSIM_ID d, NETSIM_ID in,
							   NetSim_PACKET* packet, double time)
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
	pevent.nEventType = NETWORK_IN_EVENT;
	pevent.nPacketId = packet->nPacketId;
	pevent.nProtocolId = fn_NetSim_Stack_GetNWProtocol(d);
	pevent.pPacket = packet;
	fnpAddEvent(&pevent);
}

void satellite_handle_mac_in()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	double time = pstruEventDetails->dEventTime;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	SATELLITE_HDR_REMOVE(packet);
	forward_to_network(d, in, packet, time);
}
#pragma endregion

#pragma region BUFFER_LIST
static void form_bufferlist_for_forwardlink(NETSIM_ID d, NETSIM_ID in,
											ptrSUPERFRAME sf)
{
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;
	NETSIM_ID g = 0, gin = 0;
	NETSIM_ID i, j;
	for (i = 0; i < link->puniDevList.pstrup2MP.nConnectedDeviceCount-1; i++)
	{
		g = link->puniDevList.pstrup2MP.anDevIds[i];
		gin = link->puniDevList.pstrup2MP.anDevInterfaceIds[i];
		ptrSATELLITE_PROTODATA pd = DEVICE_MACVAR(g, gin);
		if (pd->deviceType == SATELLITE_DEVICETYPE_SATELLITE_GATEWAY)
		{
			ptrSATELLITE_GW_MAC mac = SATELLITE_GWMAC_GET(g, gin);
			for (j = 0; j < mac->nAssocUTCount; j++)
			{
				if (sf->bufferCount)
					sf->buffers = realloc(sf->buffers, (sf->bufferCount + 1) * (sizeof * sf->buffers));
				else
					sf->buffers = calloc(1, sizeof * sf->buffers);
				sf->buffers[sf->bufferCount] = mac->utAssocInfo[j]->buffer;
				sf->bufferCount++;
			}
		}
	}
}

static void form_bufferlist_for_returnlink(NETSIM_ID d, NETSIM_ID in,
											ptrSUPERFRAME sf)
{
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;
	NETSIM_ID u, uin;
	NETSIM_ID i;
	for (i = 0; i < link->puniDevList.pstrup2MP.nConnectedDeviceCount - 1; i++)
	{
		u = link->puniDevList.pstrup2MP.anDevIds[i];
		uin = link->puniDevList.pstrup2MP.anDevInterfaceIds[i];
		ptrSATELLITE_PROTODATA pd = DEVICE_MACVAR(u, uin);
		if (pd->deviceType == SATELLITE_DEVICETYPE_SATELLITE_GATEWAY)
			continue;

		ptrSATELLITE_UT_MAC mac = SATELLITE_UTMAC_GET(u, uin);
		if (sf->bufferCount)
			sf->buffers = realloc(sf->buffers, (sf->bufferCount + 1) * (sizeof * sf->buffers));
		else
			sf->buffers = calloc(1, sizeof * sf->buffers);
		sf->buffers[sf->bufferCount] = mac->buffer;
		sf->bufferCount++;
	}
}

void satellite_form_bufferList(NETSIM_ID d, NETSIM_ID in,
							   ptrSUPERFRAME sf)
{
	if (sf->linkType == LINKTYPE_FORWARD)
		form_bufferlist_for_forwardlink(d, in, sf);
	else if (sf->linkType == LINKTYPE_RETURN)
		form_bufferlist_for_returnlink(d, in, sf);
	else
	{
		fnNetSimError("Unknwon link type %d in function %s\n",
					  sf->linkType, __FUNCTION__);
	}
}
#pragma endregion