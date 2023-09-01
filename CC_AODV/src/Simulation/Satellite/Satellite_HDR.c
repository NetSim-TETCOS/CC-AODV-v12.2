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
#include "Satellite_HDR.h"
#include "Satellite_MAC.h"

void satellite_hdr_init(NETSIM_ID d, NETSIM_ID in,
						NetSim_PACKET* packet)
{
	ptrSATELLITE_HDR hdr = calloc(1, sizeof * hdr);
	
	packet->pstruMacData->nMACProtocol = MAC_PROTOCOL_SATELLITE;
	packet->pstruMacData->Packet_MACProtocol = hdr;

	if (isUT(d, in))
	{
		ptrSATELLITE_UT_MAC mac = SATELLITE_UTMAC_GET(d, in);
		hdr->gwId = mac->gatewayId;
		hdr->gwIf = mac->gatewayIf;
		hdr->linkType = LINKTYPE_RETURN;
		hdr->satelliteId = mac->satelliteId;
		hdr->satelliteIf = mac->satelliteIf;
		hdr->utId = d;
		hdr->utIf = in;
	}
	else if (isGW(d, in))
	{
		ptrSATELLITE_GW_MAC mac = SATELLITE_GWMAC_GET(d, in);
		hdr->gwId = d;
		hdr->gwIf = in;
		hdr->linkType = LINKTYPE_FORWARD;
		hdr->satelliteId = mac->satelliteId;
		hdr->satelliteIf = mac->satelliteIf;
		hdr->utId = packet->nReceiverId;
		hdr->utIf = fn_NetSim_Stack_GetConnectedInterface(d, in, hdr->utId);
	}
	else
	{
		fnNetSimError("Unknown device type %d for %d:%d in function %s\n",
					  SATELLITE_DEVICETYPE_GET(d, in), d, in);
	}
}

ptrSATELLITE_HDR SATELLITE_HDR_GET(NetSim_PACKET* packet)
{
	return (ptrSATELLITE_HDR)packet->pstruMacData->Packet_MACProtocol;
}

void SATELLITE_HDR_REMOVE(NetSim_PACKET* packet)
{
	ptrSATELLITE_HDR hdr = packet->pstruMacData->Packet_MACProtocol;
	free(hdr);
	packet->pstruMacData->Packet_MACProtocol = NULL;
}
