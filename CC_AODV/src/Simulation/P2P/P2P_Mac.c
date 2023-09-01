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
#include "P2P.h"
#include "P2P_Enum.h"

int P2P_MacOut_Handler()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;

	NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(d, in);
	NetSim_PACKET* pstruPacket;
	
	if (pstruEventDetails->nSubEventType == P2P_MAC_IDLE)
	{
		P2P_MAC_SET_IDLE(d, in);
		pstruEventDetails->nSubEventType = 0;
	}

	if (P2P_MAC_IS_BUSY(d, in)) return -1; // Mac is busy.

	//Get the packet from the Interface buffer
	pstruPacket = fn_NetSim_Packet_GetPacketFromBuffer(buf, 1);
	if (!pstruPacket)	return -2;

	fnValidatePacket(pstruPacket);

	P2P_MAC_SET_BUSY(d, in);

	//Place the packet to physical layer
	//Write physical out event.
	//Append mac details in packet
	pstruPacket->pstruMacData->dArrivalTime = ldEventTime;
	pstruPacket->pstruMacData->dEndTime = ldEventTime;
	if (pstruPacket->pstruNetworkData)
	{
		pstruPacket->pstruMacData->dOverhead = 0;
		pstruPacket->pstruMacData->dPayload = pstruPacket->pstruNetworkData->dPacketSize;
		pstruPacket->pstruMacData->dPacketSize = pstruPacket->pstruMacData->dOverhead +
			pstruPacket->pstruMacData->dPayload;
	}
	if (pstruPacket->pstruMacData->Packet_MACProtocol &&
		!pstruPacket->pstruMacData->dontFree)
	{
		fn_NetSim_Packet_FreeMacProtocolData(pstruPacket);
		pstruPacket->pstruMacData->Packet_MACProtocol = NULL;
	}
	pstruPacket->pstruMacData->nMACProtocol = pstruEventDetails->nProtocolId;
	pstruPacket->pstruMacData->dStartTime = ldEventTime;
	//Update the event details
	pstruEventDetails->dEventTime = ldEventTime;
	pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;
	pstruEventDetails->pPacket = pstruPacket;
	pstruEventDetails->nPacketId = pstruPacket->nPacketId;
	if (pstruPacket->pstruAppData)
	{
		pstruEventDetails->nApplicationId = pstruPacket->pstruAppData->nApplicationId;
		pstruEventDetails->nSegmentId = pstruPacket->pstruAppData->nSegmentId;
	}
	else
	{
		pstruEventDetails->nApplicationId = 0;
		pstruEventDetails->nSegmentId = 0;
	}
	//Add physical out event
	fnpAddEvent(pstruEventDetails);
	return 0;
}

int P2P_MacIn_Handler()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	if (DEVICE_NWLAYER(d))
	{
		//Add packet to Network in
		//Prepare event details for network in
		pstruEventDetails->dEventTime = ldEventTime;
		pstruEventDetails->nEventType = NETWORK_IN_EVENT;
		pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetNWProtocol(d);
		//Add network in event
		fnpAddEvent(pstruEventDetails);
	}
	else
	{
		bool isTransmitted = false;
		//Broadcast to all other interface
		NETSIM_ID i;
		for (i = 0; i < DEVICE(d)->nNumOfInterface; i++)
		{
			if (i + 1 == in)
				continue;

			NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(d, i + 1);
			//Check for buffer
			if (!fn_NetSim_GetBufferStatus(buf))
			{
				//Prepare event details for mac out
				pstruEventDetails->dEventTime = ldEventTime;
				pstruEventDetails->nEventType = MAC_OUT_EVENT;
				pstruEventDetails->nInterfaceId = i + 1;
				pstruEventDetails->pPacket = NULL;
				pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetMacProtocol(d, i + 1);
				//Add mac out event
				fnpAddEvent(pstruEventDetails);
			}
			//Add packet to mac buffer
			fn_NetSim_Packet_AddPacketToList(buf,
											 isTransmitted == false ? packet : fn_NetSim_Packet_CopyPacket(packet), 0);
			isTransmitted = true;
		}
	}
	return 0;
}
