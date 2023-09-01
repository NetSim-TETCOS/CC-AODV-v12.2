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
#include "IEEE802_11.h"
#include "IEEE802_11_MAC_Frame.h"
#include "IEEE802_11_Phy.h"

//Function prototype
static void fn_NetSim_IEEE802_11_MacInit(bool* isInterfaceUsed);
static void forward_to_other_interface(NetSim_PACKET* packet);
static void add_to_mac_queue(NetSim_PACKET* packet);

void fn_NetSim_IEEE802_11_MacOut()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID ifCount = DEVICE(d)->nNumOfInterface;
	NETSIM_ID i;

	switch(pstruEventDetails->nSubEventType)
	{
	case 0:
	{
		bool* isInterfaceUsed = calloc(ifCount, sizeof * isInterfaceUsed);
		fn_NetSim_IEEE802_11_MacInit(isInterfaceUsed);
		for (i = 0; i < ifCount; i++)
		{
			if (isInterfaceUsed[i])
			{
				pstruEventDetails->nInterfaceId = i + 1;
				fn_NetSim_IEEE802_11_CSMACA_Init();
			}
		}
		free(isInterfaceUsed);
	}
	break;
	case CS:
		if(fn_NetSim_IEEE802_11_CSMACA_CS())
			fn_NetSim_IEEE802_11_CSMACA_CheckNAV();
		break;
	case IEEE802_11_EVENT_DIFS_FAILED:
		ieee802_11_csmaca_difs_failed(IEEE802_11_CURR_MAC);
		break;
	case IEEE802_11_EVENT_DIFS_END:
		fn_NetSim_IEEE802_11_CSMACA_DIFSEnd();
		break;
	case IEEE802_11_EVENT_AIFS_FAILED:
		ieee802_11_csmaca_aifs_failed(IEEE802_11_CURR_MAC);
		break;
	case IEEE802_11_EVENT_AIFS_END:
		fn_NetSim_IEEE802_11_CSMACA_AIFSEnd();
		break;
	case IEEE802_11_EVENT_BACKOFF:
		if(fn_NetSim_IEEE802_11_CSMACA_Backoff())
			fn_NetSim_IEEE802_11_SendToPhy();
		break;
	case IEEE802_11_EVENT_BACKOFF_PAUSED:
		ieee802_11_csmaca_pause_backoff(IEEE802_11_CURR_MAC);
		break;
	case SEND_ACK:
		if(IEEE802_11_CURR_MAC->macAggregationStatus)
			fn_NetSim_IEEE802_11_CSMACA_SendBlockACK();
		else
			fn_NetSim_IEEE802_11_CSMACA_SendACK();
		break;
	case SEND_CTS:
		fn_NetSim_IEEE802_11_RTS_CTS_SendCTS();
		break;
	case SEND_MPDU:
		fn_NetSim_IEEE802_11_SendToPhy();
		break;
	default:
		fnNetSimError("Unknown subevent %d for IEEE802_11 MAC OUT.",pstruEventDetails->nSubEventType);
		break;
	}
}

int fn_NetSim_IEEE802_11_MacIn()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	PIEEE802_11_MAC_VAR mac = IEEE802_11_CURR_MAC;
	NetSim_EVENTDETAILS pevent;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NetSim_PACKET* data;
	
	if(isIEEE802_11_CtrlPacket(packet))
	{
		fn_NetSim_Process_CtrlPacket();
		return 0;
	}

	mac->metrics.nReceivedFrameCount++;

	memcpy(&pevent,pstruEventDetails,sizeof pevent);
	data = fn_NetSim_Packet_CopyPacket(packet);
	if(DEVICE_NWLAYER(d))
	{
		pevent.nInterfaceId = mac->parentInterfaceId;
		pevent.nSubEventType=0;
		pevent.nProtocolId = fn_NetSim_Stack_GetNWProtocol(d);
		pevent.nEventType=NETWORK_IN_EVENT;
		fnpAddEvent(&pevent);
	}
	else
	{
		if(mac->BSSType==INFRASTRUCTURE)
		{
			if (isBroadcastPacket(packet) || isMulticastPacket(packet))
			{
				NetSim_PACKET* p = fn_NetSim_Packet_CopyPacket(packet);
				fn_NetSim_802_11_InfrastructureBSS_UpdateReceiver(packet);
				add_to_mac_queue(packet);
				forward_to_other_interface(p);
			}
			else
			{
				if (isPacketforsameInfrastructureBSS(mac, packet))
				{
					fn_NetSim_802_11_InfrastructureBSS_UpdateReceiver(packet);
					add_to_mac_queue(packet);
				}
				else
					forward_to_other_interface(packet);
			}
		}
		else if(mac->BSSType==MESH)
		{
			if(isPacketforsameMeshBSS(mac,packet))
			{
				fn_NetSim_802_11_MeshBSS_UpdateReceiver(packet);
				add_to_mac_queue(packet);
			}
			else
				forward_to_other_interface(packet);
		}
		else
		{
			fnNetSimError("BSSType %d is not implemented for IEEE802.11 protocol");
			return -1;
		}
	}
	if (!isBroadcastPacket(data) &&
		!isMulticastPacket(data)) //No ack for broadcast or multicast
	{
		memcpy(&pevent, pstruEventDetails, sizeof pevent);
		
		//Add event to send ack
		pevent.nEventType=MAC_OUT_EVENT;
		pevent.nSubEventType=SEND_ACK;
		pevent.pPacket=data;
		fnpAddEvent(&pevent);
		IEEE802_11_Change_Mac_State(IEEE802_11_CURR_MAC,IEEE802_11_MACSTATE_TXing_ACK);
	}
	return 0;
}

static void fn_NetSim_IEEE802_11_MacInit(bool* isInterfaceUsed)
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	PIEEE802_11_MAC_VAR mac=IEEE802_11_CURR_MAC;
	NetSim_BUFFER* pstruBuffer = DEVICE_ACCESSBUFFER(d, in);

	if (mac->currentProcessingPacket)
	{
		isInterfaceUsed[in - 1] = true;
	}

	if (mac->parentInterfaceId != in)
	{
		isInterfaceUsed[in - 1] = true;
	}

	if (isPacketInQueue(d, in))
		isInterfaceUsed[in - 1] = true;

	while(fn_NetSim_GetBufferStatus(pstruBuffer))
	{
		NetSim_PACKET* pstruPacket = fn_NetSim_Packet_GetPacketFromBuffer(pstruBuffer,1);

		if(mac->BSSType==INFRASTRUCTURE)
		{
			if(isPacketforsameInfrastructureBSS(mac,pstruPacket))
				fn_NetSim_802_11_InfrastructureBSS_UpdateReceiver(pstruPacket);
			else
			{
				fn_NetSim_Packet_FreePacket(pstruPacket);
				continue;
			}
		}
		else if(mac->BSSType==MESH)
		{
			if(isPacketforsameMeshBSS(mac,pstruPacket))
				fn_NetSim_802_11_MeshBSS_UpdateReceiver(pstruPacket);
			else
			{
				fn_NetSim_Packet_FreePacket(pstruPacket);
				continue;
			}
		}
		else
		{
			fnNetSimError("BSSType %d is not implemented for IEEE802.11 protocol", mac->BSSType);
			return;
		}

		pstruPacket->pstruMacData->dArrivalTime = pstruEventDetails->dEventTime	;
		pstruPacket->pstruMacData->dPayload = pstruPacket->pstruNetworkData->dPacketSize;
		
		//Call IEEE802.11e
		NETSIM_ID vin = add_to_queue(d, in, pstruPacket);
		isInterfaceUsed[vin - 1] = true;
	}
	return;
}

void IEEE802_11_Change_Mac_State(PIEEE802_11_MAC_VAR mac,IEEE802_11_MAC_STATE state)
{
	print_ieee802_11_log("Device %d, interface %d, changing mac state from %s to %s.",
						 mac->deviceId,
						 mac->interfaceId,
						 strIEEE802_11_MAC_STATE[mac->currMacState],
						 strIEEE802_11_MAC_STATE[state]);
	mac->prevMacState=mac->currMacState;
	mac->currMacState=state;
}

void set_mac_state_after_txend(PIEEE802_11_MAC_VAR mac)
{
	switch(mac->currMacState)
	{
	case IEEE802_11_MACSTATE_TXing_ACK:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_MAC_IDLE);
		break;
	case IEEE802_11_MACSTATE_TXing_MPDU:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_Wait_ACK);
		break;
	case IEEE802_11_MACSTATE_Txing_BroadCast:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_MAC_IDLE);
		break;
	case IEEE802_11_MACSTATE_TXing_CTS:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_MAC_IDLE);
		break;
	case IEEE802_11_MACSTATE_TXing_RTS:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_Wait_CTS);
		break;
	case IEEE802_11_MACSTATE_MAC_IDLE:
		if (mac->prevMacState == IEEE802_11_MACSTATE_Txing_BroadCast)
			break; // Ignore the multiple change mac state due to broadcast transmission.
	default:
		fnNetSimError("Unknown mac state %d is %s",mac->currMacState,__FUNCTION__);
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_MAC_IDLE);
	}
}

void set_mac_state_for_tx(PIEEE802_11_MAC_VAR mac,NetSim_PACKET* p)
{
	switch(p->nControlDataType)
	{
	case WLAN_RTS:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_TXing_RTS);
		break;
	case WLAN_CTS:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_TXing_CTS);
		break;
	case WLAN_ACK:
		IEEE802_11_Change_Mac_State(mac,IEEE802_11_MACSTATE_TXing_ACK);
		break;
	default:
		if(isBroadcastPacket(p) || isMulticastPacket(p))
			IEEE802_11_Change_Mac_State(mac, IEEE802_11_MACSTATE_Txing_BroadCast);
		else
			IEEE802_11_Change_Mac_State(mac, IEEE802_11_MACSTATE_TXing_MPDU);
		break;
	}
}

bool isMacTransmittingState(PIEEE802_11_MAC_VAR mac)
{
	if (mac->currMacState == IEEE802_11_MACSTATE_TXing_MPDU ||
		mac->currMacState == IEEE802_11_MACSTATE_Txing_BroadCast ||
		mac->currMacState == IEEE802_11_MACSTATE_TXing_ACK ||
		mac->currMacState == IEEE802_11_MACSTATE_TXing_RTS ||
		mac->currMacState == IEEE802_11_MACSTATE_TXing_CTS)
		return true;
	return false;
}

bool isMacReceivingState(PIEEE802_11_MAC_VAR mac)
{
	if (mac->currMacState == IEEE802_11_MACSTATE_Wait_ACK ||
		mac->currMacState == IEEE802_11_MACSTATE_Wait_CTS ||
		mac->currMacState == IEEE802_11_MACSTATE_Wait_BlockACK ||
		mac->currMacState == IEEE802_11_MACSTATE_Wait_DATA)
		return true;
	return false;
}

bool isMacIdle(PIEEE802_11_MAC_VAR mac)
{
	if (mac->currMacState == IEEE802_11_MACSTATE_MAC_IDLE ||
		mac->currMacState == IEEE802_11_MACSTATE_WF_NAV ||
		mac->currMacState == IEEE802_11_MACSTATE_Wait_DIFS ||
		mac->currMacState == IEEE802_11_MACSTATE_Wait_AIFS ||
		mac->currMacState == IEEE802_11_MACSTATE_BACKING_OFF)
		return true;
	return false;
}

void fn_NetSim_IEEE802_11_SendToPhy()
{
	double dPacketSize=0;	
	NETSIM_ID nDeviceId = pstruEventDetails->nDeviceId;
	NETSIM_ID nInterfaceId = pstruEventDetails->nInterfaceId;
	PIEEE802_11_MAC_VAR mac = IEEE802_11_CURR_MAC;
	
	NetSim_PACKET *p=mac->currentProcessingPacket;
	NetSim_PACKET* pstruPacket;
	unsigned int i=1;

	if(!mac->currentProcessingPacket)
	{
		fnNetSimError("%s is called without mac->currentProcessingPacket",__FUNCTION__);
		return;
	}
	set_mac_state_for_tx(mac,p);
	ieee802_11_edcaf_set_txop_time(mac, ldEventTime);

	if(!isBroadcastPacket(mac->currentProcessingPacket) &&
	   !isMulticastPacket(mac->currentProcessingPacket))
		pstruPacket = fn_NetSim_Packet_CopyPacketList(mac->currentProcessingPacket);
	else
	{
		pstruPacket = mac->currentProcessingPacket;
		mac->currentProcessingPacket = NULL;
	}
	
	pstruEventDetails->pPacket = pstruPacket;
	// Add the MAC header
	while(pstruPacket)
	{
		if(!isIEEE802_11_CtrlPacket(pstruPacket))
		{
			fn_NetSim_IEEE802_11_Add_MAC_Header(nDeviceId,nInterfaceId,pstruPacket,i);
			mac->metrics.nTransmittedFrameCount++;
		}
		dPacketSize += pstruPacket->pstruMacData->dPacketSize;
		pstruPacket=pstruPacket->pstruNextPacket;
		i++;
	}
	pstruPacket = pstruEventDetails->pPacket;
	pstruPacket->nTransmitterId = pstruEventDetails->nDeviceId;

	pstruEventDetails->nInterfaceId = mac->parentInterfaceId;
	pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;	
	pstruEventDetails->nPacketId = pstruPacket->nPacketId;
	if(pstruPacket->pstruAppData)
	{
		pstruEventDetails->nSegmentId = pstruPacket->pstruAppData->nSegmentId;
		pstruEventDetails->nApplicationId = pstruPacket->pstruAppData->nApplicationId;
	}
	else
	{
		pstruEventDetails->nApplicationId = 0;
		pstruEventDetails->nSegmentId = 0;
	}
	pstruEventDetails->nSubEventType = IEEE802_11_PHY_TXSTART_REQUEST;
	pstruEventDetails->dPacketSize = dPacketSize;
	fnpAddEvent(pstruEventDetails);
}

static void forward_to_other_interface(NetSim_PACKET* packet)
{
	NetSim_EVENTDETAILS pevent;
	NETSIM_ID i;
	NETSIM_ID inf=pstruEventDetails->nInterfaceId;
	NETSIM_ID d=pstruEventDetails->nDeviceId;

	fn_NetSim_IEEE802_11_FreePacket(packet);
	packet->pstruMacData->nMACProtocol = MAC_PROTOCOL_NULL;
	packet->pstruMacData->Packet_MACProtocol=NULL;
	packet->pstruMacData->dPacketSize=0;
	packet->pstruMacData->dPayload=0;
	packet->pstruMacData->dOverhead=0;
	for(i=1;i<=DEVICE(d)->nNumOfInterface;i++)
	{
		NetSim_BUFFER* buf=DEVICE_ACCESSBUFFER(d,i);
		NetSim_PACKET* p;

		if(i==inf)
			continue;

		if (isVirtualInterface(d, i)) continue;

		if(!fn_NetSim_GetBufferStatus(buf))
		{
			memcpy(&pevent, pstruEventDetails, sizeof pevent);
			pevent.nInterfaceId=i;
			pevent.nEventType=MAC_OUT_EVENT;
			pevent.nProtocolId=fn_NetSim_Stack_GetMacProtocol(d,i);
			pevent.nSubEventType=0;
			pevent.pPacket=NULL;
			fnpAddEvent(&pevent);
		}
		p=fn_NetSim_Packet_CopyPacket(packet);
		fn_NetSim_Packet_AddPacketToList(buf,p,0);
	}
	fn_NetSim_Packet_FreePacket(packet);
}

static void add_to_mac_queue(NetSim_PACKET* packet)
{
	NETSIM_ID inf=pstruEventDetails->nInterfaceId;
	NETSIM_ID d=pstruEventDetails->nDeviceId;
	NetSim_BUFFER* buf=DEVICE_ACCESSBUFFER(d,inf);

	fn_NetSim_IEEE802_11_FreePacket(packet);
	packet->pstruMacData->nMACProtocol = MAC_PROTOCOL_NULL;
	packet->pstruMacData->Packet_MACProtocol=NULL;
	packet->pstruMacData->dPacketSize=0;
	packet->pstruMacData->dPayload=0;
	packet->pstruMacData->dOverhead=0;

	if(!fn_NetSim_GetBufferStatus(buf))
	{
		pstruEventDetails->nInterfaceId=inf;
		pstruEventDetails->nEventType=MAC_OUT_EVENT;
		pstruEventDetails->nProtocolId=fn_NetSim_Stack_GetMacProtocol(d,inf);
		pstruEventDetails->nSubEventType=0;
		pstruEventDetails->pPacket=NULL;
		fnpAddEvent(pstruEventDetails);
	}
	fn_NetSim_Packet_AddPacketToList(buf,packet,0);
}

void fn_NetSim_IEE802_11_MacReInit(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId)
{
	NetSim_EVENTDETAILS pevent;
	memcpy(&pevent,pstruEventDetails,sizeof pevent);
	pstruEventDetails->dEventTime += 0.1;
	pstruEventDetails->nDeviceId=nDeviceId;
	pstruEventDetails->nInterfaceId=nInterfaceId;
	pstruEventDetails->nDeviceType=DEVICE_TYPE(nDeviceId);
	pstruEventDetails->dPacketSize=0;
	pstruEventDetails->nApplicationId=0;
	pstruEventDetails->nEventType=MAC_OUT_EVENT;
	pstruEventDetails->nPacketId=0;
	pstruEventDetails->nSegmentId=0;
	pstruEventDetails->nSubEventType=0;
	pstruEventDetails->pPacket=NULL;
	pstruEventDetails->szOtherDetails=NULL;
	IEEE802_11_CURR_MAC->nRetryCount = 0;
	fn_NetSim_IEEE802_11_CSMACA_Init();
	memcpy(pstruEventDetails,&pevent,sizeof pevent);
}

double calculate_nav(NETSIM_ID d, NETSIM_ID i, NetSim_PACKET* packet)
{
	PIEEE802_11_PHY_VAR phy = IEEE802_11_PHY(d, i);
	double nav = 0;
	switch (packet->nControlDataType)
	{
	case WLAN_RTS:
	{
		double duration = (double)(((PIEEE802_11_RTS)(packet->pstruMacData->Packet_MACProtocol))->Duration);
		nav += ceil(packet->pstruPhyData->dStartTime + duration) + 3;
	}
	break;
	case WLAN_CTS:
	{
		double duration = (double)(((PIEEE802_11_CTS)(packet->pstruMacData->Packet_MACProtocol))->Duration);
		nav += ceil(packet->pstruPhyData->dStartTime + duration) + 2;
	}
	break;
	case WLAN_ACK:
		nav += ceil(packet->pstruPhyData->dStartTime);
		break;
	case WLAN_BlockACK:
		nav += ceil(packet->pstruPhyData->dStartTime);
		break;
	default:
		if (isBroadcastPacket(packet) ||
			isMulticastPacket(packet))
		{
			nav += ceil(packet->pstruPhyData->dStartTime) + 1;
		}
		else
		{
			nav += ceil(packet->pstruPhyData->dStartTime
						+ phy->plmeCharacteristics.aSIFSTime
						+ get_preamble_time(phy)
						+ ((getAckSize(phy) * 8) / phy->dControlFrameDataRate)) + 1;
		}
		break;
	}
	return nav;
}