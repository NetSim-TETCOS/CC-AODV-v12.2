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

#include "stdafx.h"
#include "LTENR_EPC.h"

#pragma region EPC_INIT
void LTENR_EPC_ALLOC(NETSIM_ID d, NETSIM_ID in)
{
	ptrLTENR_EPC epc = calloc(1, sizeof * epc);
	LTENR_EPC_SET(d, in, epc);
	epc->d = d;
	epc->in = in;
}
#pragma endregion

#pragma region EPC_ASSOCIATION
void LTENR_EPC_ASSOCIATION(NETSIM_ID epcId, NETSIM_ID epcIf,
						   NETSIM_ID gnbId, NETSIM_ID gnbIf,
						   NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_EPC_HLR hlr = LTENR_EPC_HLR_ALLOC();
	hlr->gnbId = gnbId;
	hlr->gnbIf = gnbIf;
	hlr->ueId = ueId;
	hlr->ueIf = ueIf;

	ptrLTENR_EPC epc = LTENR_EPC_GET(epcId, epcIf);
	LTENR_EPC_HLR_ADD(epc, hlr);
}

void LTENR_GNB_SETEPC(NETSIM_ID d, NETSIM_ID in,
					   NETSIM_ID* e, NETSIM_ID* ein)
{
	NETSIM_ID i;
	NETSIM_ID r;
	NETSIM_ID rin;

	for (i = 0; i < DEVICE(d)->nNumOfInterface; i++)
	{
		if (i + 1 == in)
			continue;

		fn_NetSim_Stack_GetConnectedDevice(d, i + 1, &r, &rin);
		if (DEVICE_INTERFACE(r, rin)->nLocalNetworkProtocol == MAC_PROTOCOL_LTE_NR ||
			DEVICE_INTERFACE(r, rin)->nLocalNetworkProtocol == MAC_PROTOCOL_LTE)
		{
			*e = r;
			*ein = rin;
			break;
		}
	}
}

ptrLTENR_EPC_HLR LTENR_EPC_FindHLR(NETSIM_ID d, NETSIM_ID in, NETSIM_ID dest)
{
	ptrLTENR_EPC epc = LTENR_EPC_GET(d, in);
	ptrLTENR_EPC_HLR hlr = epc->hlr;
	while (hlr)
	{
		if (hlr->ueId == dest)
			return hlr;
		LTENR_EPC_HLR_NEXT(hlr);
	}
	return NULL;
}

NETSIM_ID LTENR_EPC_HLR_FindOutInterface(NETSIM_ID d, NETSIM_ID dest)
{
	NETSIM_ID i;
	for (i = 0; i < DEVICE(d)->nNumOfInterface; i++)
	{
		if (DEVICE_INTERFACE(d, i + 1)->nLocalNetworkProtocol == MAC_PROTOCOL_LTE_NR ||
			DEVICE_INTERFACE(d, i + 1)->nLocalNetworkProtocol == MAC_PROTOCOL_LTE)
		{
			if(LTENR_EPC_FindHLR(d, i + 1, dest))
				return i + 1;
		}
	}
	return 0;
}
#pragma endregion

#pragma region EPC_PACKETPROCESSING
void LTENR_EPC_NetworkOut()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	//NETSIM_ID in = pstruEventDetails->nInterfaceId;

	if (isBroadcastPacket(packet) ||
		isMulticastPacket(packet))
	{
		//No broadcast and multicast
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	NETSIM_ID dest = get_first_dest_from_packet(packet);
	NETSIM_ID outIn = LTENR_EPC_HLR_FindOutInterface(d, dest);
	if (!outIn)
	{
		print_ltenr_log("Device %d is not connected to EPC %d\n", dest, d);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(d, outIn);
	if (!fn_NetSim_GetBufferStatus(buf))
	{
		pstruEventDetails->nEventType = MAC_OUT_EVENT;
		pstruEventDetails->nInterfaceId = outIn;
		pstruEventDetails->nProtocolId = DEVICE_MACLAYER(d, outIn)->nMacProtocolId;
		pstruEventDetails->pPacket = NULL;
		fnpAddEvent(pstruEventDetails);
	}
	fn_NetSim_Packet_AddPacketToList(buf, packet, 0);
	pstruEventDetails->pPacket = NULL;
}
#pragma endregion
