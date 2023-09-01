/************************************************************************************
* Copyright (C) 2020                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                       *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "main.h"
#include "AODV.h"
#include "List.h"
static void init_aodv_state();

int fn_NetSim_AODV_Init_F()
{
	NETSIM_ID loop;

	init_aodv_state();
	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer && NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer->nRoutingProtocolId == NW_PROTOCOL_AODV)
		{
			NetSim_EVENTDETAILS pevent;
			pevent.dEventTime = AODV_HELLO_INTERVAL;
			pevent.dPacketSize=0;
			pevent.nApplicationId=0;
			pevent.nDeviceId=loop+1;
			pevent.nDeviceType=NETWORK->ppstruDeviceList[loop]->nDeviceType;
			pevent.nEventType = TIMER_EVENT;
			pevent.nInterfaceId=0;
			pevent.nProtocolId = NW_PROTOCOL_AODV;
			pevent.nSegmentId=0;
			pevent.nSubEventType = AODVsubevent_TRANSMIT_HELLO;
			pevent.nPacketId=0;
			pevent.pPacket =NULL;
			pevent.szOtherDetails = NULL;
			fnpAddEvent(&pevent);
		}
	}
	return 1;
}
char* fn_NetSim_AODV_Trace_F(NETSIM_ID id)
{
	switch(id)
	{
	case AODVsubevent_RREQ_TIMEOUT:
		return "AODV_RREQ_TIME_OUT";
	case AODVsubevent_TRANSMIT_HELLO:
		return "AODV_TRANSMIT_HELLO";
	case AODVsubevent_ACTIVE_ROUTE_TIMEOUT:
		return "AODV_ACTIVE_ROUTE_TIMEOUT";
	default:
		return "AODV_UNKNOWN";
	}
}
int fn_NetSim_AODV_FreePacket_F(NetSim_PACKET* packet)
{
	switch(packet->nControlDataType)
	{
	case AODVctrlPacket_RREQ:
		{
			AODV_RREQ* rreq = packet->pstruNetworkData->Packet_RoutingProtocol;
			IP_FREE(rreq->DestinationIPAddress);
			IP_FREE(rreq->LastAddress);
			IP_FREE(rreq->OriginatorIPAddress);
			free(rreq);
		}
		break;
	case AODVctrlPacket_RREP:
		{
			AODV_RREP* rrep = packet->pstruNetworkData->Packet_RoutingProtocol;
			IP_FREE(rrep->DestinationIPaddress);
			IP_FREE(rrep->LastAddress);
			IP_FREE(rrep->OriginatorIPaddress);
			free(rrep);
		}
		break;
	case AODVctrlPacket_RERR:
		{
			unsigned int loop;
			AODV_RERR* rerr = packet->pstruNetworkData->Packet_RoutingProtocol;
			for(loop=0;loop<rerr->DestCount;loop++)
			{
				IP_FREE(rerr->UnreachableDestinationIPAddress[loop]);
			}
			free(rerr->UnreachableDestinationIPAddress);
			free(rerr->UnreachableDestinationSequenceNumber);
			free(rerr);
		}
		break;
	}
	return 1;
}
int fn_NetSim_AODV_CopyPacket_F(const NetSim_PACKET* destPacket,const NetSim_PACKET* srcPacket)
{
	switch(srcPacket->nControlDataType)
	{
	case AODVctrlPacket_RREQ:
		{
			AODV_RREQ* src = srcPacket->pstruNetworkData->Packet_RoutingProtocol;
			AODV_RREQ* dest = calloc(1,sizeof* dest);
			dest->DestinationIPAddress = IP_COPY(src->DestinationIPAddress);
			dest->DestinationSequenceNumber = src->DestinationSequenceNumber;
			dest->HopCount = src->HopCount;
			strcpy(dest->JRGDU,src->JRGDU);
			dest->LastAddress = IP_COPY(src->LastAddress);
			dest->OriginatorIPAddress = IP_COPY(src->OriginatorIPAddress);
			dest->OriginatorSequenceNumber = src->OriginatorSequenceNumber;
			dest->Reserved = src->Reserved;
			dest->RREQID = src->RREQID;
			dest->Type = src->Type;
			destPacket->pstruNetworkData->Packet_RoutingProtocol = dest;
		}
		break;
	case AODVctrlPacket_RREP:
		{
			AODV_RREP* src = srcPacket->pstruNetworkData->Packet_RoutingProtocol;
			AODV_RREP* dest = (AODV_RREP*)calloc(1,sizeof* dest);
			dest->DestinationIPaddress = IP_COPY(src->DestinationIPaddress);
			dest->DestinationSequenceNumber = src->DestinationSequenceNumber;
			dest->HopCount = src->HopCount;
			dest->LastAddress = IP_COPY(src->LastAddress);
			dest->Lifetime = src->Lifetime;
			dest->OriginatorIPaddress = IP_COPY(src->OriginatorIPaddress);
			dest->PrefixSz = src->PrefixSz;
			strcpy(dest->RA,src->RA);
			dest->Reserved = src->Reserved;
			dest->Type = src->Type;
			destPacket->pstruNetworkData->Packet_RoutingProtocol = dest;
		}
		break;
	case AODVctrlPacket_RERR:
		{
			unsigned int loop;
			AODV_RERR* srcErr = srcPacket->pstruNetworkData->Packet_RoutingProtocol;
			AODV_RERR* destErr = calloc(1,sizeof* destErr);
			memcpy(destErr,srcErr,sizeof* destErr);
			destPacket->pstruNetworkData->Packet_RoutingProtocol = destErr;
			destErr->UnreachableDestinationIPAddress = calloc(destErr->DestCount,sizeof* destErr->UnreachableDestinationIPAddress);
			destErr->UnreachableDestinationSequenceNumber = calloc(destErr->DestCount,sizeof* destErr->UnreachableDestinationSequenceNumber);
			for(loop=0;loop<srcErr->DestCount;loop++)
			{
				destErr->UnreachableDestinationIPAddress[loop] = IP_COPY(srcErr->UnreachableDestinationIPAddress[loop]);
				destErr->UnreachableDestinationSequenceNumber[loop] = srcErr->UnreachableDestinationSequenceNumber[loop];
			}
		}
		break;
	default:
		fnNetSimError("Unknown control packet for AODV copy packet");
		break;
	}
	return 1;
}
int fn_NetSim_AODV_Metrics_F(PMETRICSWRITER metricsWriter)
{
	NETSIM_ID loop;
	PMETRICSNODE menu = init_metrics_node(MetricsNode_Menu, "AODV Metrics", NULL);
	PMETRICSNODE table = init_metrics_node(MetricsNode_Table, "AODV Metrics", NULL);
	add_node_to_menu(menu, table);

	add_table_heading(table, "Device Id", true, 0);
	add_table_heading(table, "RREQ Sent", true, 0);
	add_table_heading(table, "RREQ forwarded", false, 0);
	add_table_heading(table, "RREP sent", false, 0);
	add_table_heading(table, "RREP forwarded", false, 0);
	add_table_heading(table, "RERR sent", false, 0);
	add_table_heading(table, "RERR forwarded", false, 0);
	add_table_heading(table, "Packet originated", false, 0);
	add_table_heading(table, "packet transmitted", false, 0);
	add_table_heading(table, "packet dropped", false, 0);

	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer && NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer->nRoutingProtocolId == NW_PROTOCOL_AODV)
		{
			add_table_row_formatted(false, table,
									"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
									loop + 1,
									AODV_METRICS_VAR(loop + 1).rreqSent,
									AODV_METRICS_VAR(loop + 1).rreqForwarded,
									AODV_METRICS_VAR(loop + 1).rrepSent,
									AODV_METRICS_VAR(loop + 1).rrepForwarded,
									AODV_METRICS_VAR(loop + 1).rerrSent,
									AODV_METRICS_VAR(loop + 1).rerrForwarded,
									AODV_METRICS_VAR(loop + 1).packetOrginated,
									AODV_METRICS_VAR(loop + 1).packetTransmitted,
									AODV_METRICS_VAR(loop + 1).packetDropped);
		}
	}
	write_metrics_node(metricsWriter, WriterPosition_Current, NULL, menu);
	return 1;
}
int fn_NetSim_AODV_Configure_F(void** var)
{
	FILE* fpConfigLog;
	void* xmlNetSimNode;
	NETSIM_ID nDeviceId;
	NETSIM_ID nInterfaceId;
	LAYER_TYPE nLayerType;
	AODV_DEVICE_VAR* deviceVar;

	fpConfigLog = var[0];
	NETWORK=var[1];
	xmlNetSimNode = var[2];
	nDeviceId = *((NETSIM_ID*)var[3]);
	nInterfaceId = *((NETSIM_ID*)var[4]);
	nLayerType = *((LAYER_TYPE*)var[5]);

	deviceVar = AODV_DEV_VAR(nDeviceId);
	if(!deviceVar)
	{
		deviceVar = calloc(1,sizeof* deviceVar);
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruNetworkLayer->RoutingVar = deviceVar;
	}
	return 1;
}
char*  fn_NetSim_AODV_ConfigPacketTrace_F()
{
	return "";
}
int fn_NetSim_AODV_Finish_F()
{
	NETSIM_ID loop;
	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer && NETWORK->ppstruDeviceList[loop]->pstruNetworkLayer->nRoutingProtocolId == NW_PROTOCOL_AODV)
		{
			AODV_DEVICE_VAR* devVar = AODV_DEV_VAR(loop+1);
			while(devVar->fifo)
			{
				IP_FREE(devVar->fifo->destination);
				while(devVar->fifo->packetList)
				{
					NetSim_PACKET* packet = devVar->fifo->packetList;
					devVar->fifo->packetList = packet->pstruNextPacket;
					packet->pstruNextPacket = NULL;
					fn_NetSim_Packet_FreePacket(packet);
				}
				LIST_FREE(&devVar->fifo,devVar->fifo);
			}
			if(devVar->precursorsList)
				while(devVar->precursorsList->count--)
					IP_FREE(devVar->precursorsList->list[devVar->precursorsList->count]);
			while(devVar->routeTable)
			{
				IP_FREE(devVar->routeTable->DestinationIPAddress);
				IP_FREE(devVar->routeTable->NextHop);
				LIST_FREE(&devVar->routeTable,devVar->routeTable);
			}
			while(devVar->rreqSeenTable)
			{
				IP_FREE(devVar->rreqSeenTable->OrginatingNode);
				LIST_FREE(&devVar->rreqSeenTable,devVar->rreqSeenTable);
			}
			while(devVar->rreqSentTable)
				LIST_FREE(&devVar->rreqSentTable,devVar->rreqSentTable);
			free(devVar);
		}
	}
	return 1;
}
char* fn_NetSim_AODV_WritePacketTrace_F()
{
	return "";
}

NetSim_PACKET* fn_NetSim_AODV_GenerateCtrlPacket(NETSIM_ID src,
	NETSIM_ID dest,
	NETSIM_ID recv,
	double dTime,
	AODV_CONTROL_PACKET type)
{
	NetSim_PACKET* packet = fn_NetSim_Packet_CreatePacket(NETWORK_LAYER);
	packet->dEventTime = dTime;
	packet->nControlDataType = type;
	add_dest_to_packet(packet, dest);
	packet->nPacketPriority = Priority_High;
	packet->nPacketType = PacketType_Control;
	packet->nQOS = QOS_BE;
	packet->nReceiverId = recv;
	packet->nSourceId = src;
	packet->nTransmitterId = src;
	packet->pstruNetworkData->dArrivalTime = dTime;
	packet->pstruNetworkData->dStartTime = dTime;
	packet->pstruNetworkData->dEndTime = dTime;
	packet->pstruNetworkData->nRoutingProtocol = NW_PROTOCOL_AODV;
	packet->pstruNetworkData->szSourceIP = aodv_get_dev_ip(src);
	packet->pstruNetworkData->szDestIP = aodv_get_dev_ip(dest);
	packet->pstruNetworkData->szGatewayIP = aodv_get_curr_ip();
	packet->pstruNetworkData->szNextHopIp = aodv_get_dev_ip(recv);
	return packet;
}
unsigned int fnFindSequenceNumber(AODV_DEVICE_VAR* devVar,NETSIM_IPAddress ip)
{
	AODV_ROUTETABLE* table = devVar->routeTable;
	while(table)
	{
		if(!IP_COMPARE(table->DestinationIPAddress,ip))
			return table->DestinationSequenceNumber;
		table = LIST_NEXT(table);
	}
	return 0;
}

NETSIM_IPAddress aodvGetdestIP(NetSim_PACKET* packet,NETSIM_ID dev)
{
	NETSIM_IPAddress src = aodv_get_dev_ip(dev);
	NETSIM_IPAddress dest = packet->pstruNetworkData->szDestIP;

	if(src->type == dest->type)
	{
		NETSIM_IPAddress n1 = IP_NETWORK_ADDRESS(src,
			DEVICE_INTERFACE(dev,1)->szSubnetMask,
			DEVICE_INTERFACE(dev,1)->prefix_len);
		NETSIM_IPAddress n2 = IP_NETWORK_ADDRESS(dest,
			DEVICE_INTERFACE(dev,1)->szSubnetMask,
			DEVICE_INTERFACE(dev,1)->prefix_len);
		if(!IP_COMPARE(n1,n2))
			return dest;
		else
		{
			if(!DEVICE_INTERFACE(dev,1)->szDefaultGateWay)
				fnNetSimError("Configure default gateway to communicate with external network for device %d\n",dev);
			return DEVICE_INTERFACE(dev,1)->szDefaultGateWay;
		}
	}
	else
	{
		if(!DEVICE_INTERFACE(dev,1)->szDefaultGateWay)
			fnNetSimError("Configure default gateway to communicate with external network for device %d\n",dev);
		return DEVICE_INTERFACE(dev,1)->szDefaultGateWay;
	}

}

static NETSIM_ID aodv_get_processing_interface(NETSIM_ID d, NetSim_PACKET* packet)
{
	if (!packet)
		return pstruEventDetails->nInterfaceId ? pstruEventDetails->nInterfaceId : 1;

	if (DEVICE(d)->nNumOfInterface == 1)
		return 1;

	NETSIM_IPAddress ip;
	NETSIM_IPAddress dest = packet->pstruNetworkData->szDestIP;

	if (isBroadcastIP(dest) || isMulticastIP(dest))
		ip = packet->pstruNetworkData->szSourceIP;
	else
		ip = packet->pstruNetworkData->szDestIP;

	NETSIM_ID i;
	for (i = 0; i < DEVICE(d)->nNumOfInterface; i++)
	{
		if (IP_IS_IN_SAME_NETWORK(ip, DEVICE_NWADDRESS(d, i + 1),
								  DEVICE_INTERFACE(d, i + 1)->szSubnetMask,
								  DEVICE_INTERFACE(d, i + 1)->prefix_len))
			return i + 1;
	}
	return 1;
}

//current aodv state
typedef struct stru_AODV_curr
{
	NETSIM_ID devId;
	NETSIM_ID ifId;
	NETSIM_IPAddress ifIP;
}AODV_CURR, *ptrAODV_CURR;

static ptrAODV_CURR aodvCurr = NULL;

void set_aodv_curr()
{
	if (!aodvCurr)
		aodvCurr = calloc(1, sizeof* aodvCurr);

	aodvCurr->devId = pstruEventDetails->nDeviceId;
	aodvCurr->ifId = aodv_get_processing_interface(aodvCurr->devId, pstruEventDetails->pPacket);
	aodvCurr->ifIP = DEVICE_NWADDRESS(aodvCurr->devId, aodvCurr->ifId);
}

NETSIM_IPAddress aodv_get_curr_ip()
{
	return aodvCurr->ifIP;
}

NETSIM_ID aodv_get_curr_if()
{
	return aodvCurr->ifId;
}

NETSIM_IPAddress aodv_get_dev_ip(NETSIM_ID d)
{
	if (d == aodvCurr->devId)
		return aodvCurr->ifIP;

	if (!d)
		return GET_BROADCAST_IP(aodvCurr->ifIP->type);

	NETSIM_ID in = fn_NetSim_Stack_GetConnectedInterface(aodvCurr->devId,
														 aodvCurr->ifId,
														 d);
	return DEVICE_NWADDRESS(d, in);
}

typedef struct stru_aodv_state
{
	NETSIM_ID d;
	NETSIM_ID in;
	struct stru_aodv_state* next;
}AODV_STATE,*ptrAODV_STATE;
ptrAODV_STATE aodvState;

static void init_aodv_state()
{
	ptrAODV_STATE state;
	ptrAODV_STATE last = NULL;
	NETSIM_ID i, j;
	for (i = 0; i < NETWORK->nDeviceCount; i++)
	{
		if (!DEVICE_NWLAYER(i + 1) ||
			DEVICE_NWLAYER(i + 1)->nRoutingProtocolId != NW_PROTOCOL_AODV)
			continue;
		for (j = 0; j < DEVICE(i + 1)->nNumOfInterface; j++)
		{
			NETSIM_ID c, ci;
			fn_NetSim_Stack_GetFirstConnectedDevice(i + 1, j + 1, &c, &ci);
			if (!DEVICE_NWLAYER(c) ||
				DEVICE_NWLAYER(c)->nRoutingProtocolId != NW_PROTOCOL_AODV)
				continue;
			state = (ptrAODV_STATE)calloc(1, sizeof* state);
			state->d = i + 1;
			state->in = j + 1;
			if (last)
				last->next = state;
			else
				aodvState = state;
			last = state;
		}
	}
}

bool isAodvConfigured(NETSIM_ID d, NETSIM_ID in)
{
	ptrAODV_STATE state = aodvState;
	while (state)
	{
		if (state->d == d && state->in == in)
			return true;
		state = state->next;
	}
	return false;
}
