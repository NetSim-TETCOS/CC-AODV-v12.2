/************************************************************************************
* Copyright (C) 2019                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#define  _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "LTENR_SDAP.h"

#pragma region SDAP_GLOBALVARIABLE
static UINT sdapDLHDRId;
static UINT sdapULHDRId;
#pragma endregion

#pragma region FUNCTION_PROTOTYPE
static void LTENR_SDAP_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
	NETSIM_ID ueId, NETSIM_ID ueIf,
	bool isAssociated);
static void LTENR_SDAP_ESTABLISHENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	NETSIM_IPAddress srcIP, NETSIM_ID srcPort,
	NETSIM_IPAddress desIP, NETSIM_ID desPort);

#pragma endregion

#pragma region FILE_CREATION_FOR_QOS
void fn_NetSim_LTE_SDAP_CreateFileDelay(UINT entityId) {
	FILE* fp = NULL;
	char filename[BUFSIZ], nodename[BUFSIZ];
	sprintf(nodename, "plot_LTENR_SDAP_Delay_Entity%d.txt", entityId);
	sprintf(filename, "%s\\%s", pszIOPath, nodename);
	fp = fopen(filename, "w+");
	if (fp)
	{
		fprintf(fp, "#Type=Line\n#Heading=Delay Vs Time\n#XLabel=Time(micro sec)\n#Num_Y=1\n#YLabel=Delay(micro sec)");
		fclose(fp);
	}
}

void fn_NetSim_LTE_SDAP_CreateFilePER(UINT entityId) {
	FILE* fp = NULL;
	char filename[BUFSIZ], nodename[BUFSIZ];
	sprintf(nodename, "plot_LTENR_SDAP_PER_Entity%d.txt", entityId);
	sprintf(filename, "%s\\%s", pszIOPath, nodename);
	fp = fopen(filename, "w+");
	if (fp)
	{
		fprintf(fp, "#Type=Line\n#Heading=PER Vs Time\n#XLabel=Time(micro sec)\n#Num_Y=1\n#YLabel=Packet Error Rate");
		fclose(fp);
	}
}

void fn_NetSim_LTE_SDAP_CreateFilePDB(UINT entityId) {
	FILE* fp = NULL;
	char filename[BUFSIZ], nodename[BUFSIZ];
	sprintf(nodename, "plot_LTENR_SDAP_PDB_Entity%d.txt", entityId);
	sprintf(filename, "%s\\%s", pszIOPath, nodename);
	fp = fopen(filename, "w+");
	if (fp)
	{
		fprintf(fp, "#Type=Line\n#Heading=PDB Vs Time\n#XLabel=Time(micro sec)\n#Num_Y=1\n#YLabel=Packet Delay Budget");
		fclose(fp);
	}
}
#pragma endregion


#pragma region SDAP_INIT
void fn_NetSim_LTENR_SDAP_Init()
{
	sdapDLHDRId = LTENR_HDR_REGISTER("SDAP_DLHDR", NULL, NULL, NULL,NULL);
	sdapULHDRId = LTENR_HDR_REGISTER("SDAP_ULHDR", NULL, NULL, NULL,NULL);

	fn_NetSim_LTENR_RegisterCallBackForAssociation(LTENR_SDAP_ASSOCIATION);

}

void fn_NetSim_LTENR_UESDAP_Init(NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_SDAPVAR sdap = calloc(1, sizeof * sdap);
	LTENR_UESDAP_SET(ueId, ueIf, sdap);

	/*ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(ueId, ueIf, 0, 0);
	while (info)
	{
		LTENR_SDAP_ESTABLISHENTITY(ueId, ueIf, info->d, info->in, 0, 0, 0, 0);//have to ask sir
		info = info->next;
	}
	*/
}

void fn_NetSim_LTENR_GNBSDAP_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_SDAPVAR sdap = calloc(1, sizeof * sdap);
	LTENR_GNBSDAP_SET(gnbId, gnbIf, sdap);

	/*ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(gnbId, gnbIf, 0, 0);
	while (info)
		LTENR_SDAP_ESTABLISHENTITY(gnbId, gnbIf, info->d, info->in, 0, 0, 0, 0);//have to ask sir
		info = info->next;
	}*/
}
#pragma endregion

#pragma region SDAP_ENTITY
static void LTENR_SDAP_ESTABLISHENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	NETSIM_IPAddress srcIP, NETSIM_ID srcPort,
	NETSIM_IPAddress desIP, NETSIM_ID desPort)
{
	ptrLTENR_SDAPVAR sdap = LTENR_SDAP_GET(d, in);
	ptrLTENR_SDAPENTITY e = LTENR_SDAPENTITY_ALLOC();
	LTENR_SDAPENTITY_ADD(sdap, e);

	e->d = d;
	e->in = in;
	e->r = r;
	e->rin = rin;
	e->srcIP = srcIP;
	e->srcPort = srcPort;
	e->desIP = desIP;
	e->desPort = desPort;
}

static ptrLTENR_SDAPENTITY LTENR_SDAP_FINDENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	NETSIM_IPAddress srcIP, NETSIM_ID srcPort,
	NETSIM_IPAddress desIP, NETSIM_ID desPort)
{
	ptrLTENR_SDAPVAR sdap = LTENR_SDAP_GET(d, in);
	ptrLTENR_SDAPENTITY e = sdap->sdapEntities;
	while (e)
	{
		if (e->r == r && e->rin == rin && e->srcIP == srcIP && e->srcPort == srcPort
			&& e->desIP == desIP && e->desPort == desPort)
			return e;
		e = LTENR_SDAPNTITY_NEXT(e);
	}
	return NULL;
}

static void LTENR_SDAP_REMOVEENTITY(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	NETSIM_IPAddress srcIP, NETSIM_ID srcPort,
	NETSIM_IPAddress desIP, NETSIM_ID desPort)
{
	ptrLTENR_SDAPVAR sdap = LTENR_SDAP_GET(d, in);
	ptrLTENR_SDAPENTITY e = LTENR_SDAP_FINDENTITY(d, in, r, rin, srcIP, srcPort, desIP, desPort);
	if (e)
		LTENR_SDAPENTITY_REMOVE(sdap, e);
}
#pragma endregion

#pragma region SDAP_ASSOCIATION
static void LTENR_SDAP_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
	NETSIM_ID ueId, NETSIM_ID ueIf,
	bool isAssociated)
{
	ptrLTENR_SDAPVAR gsdap = LTENR_GNBSDAP_GET(gnbId, gnbIf);
	if (!gsdap)
		return; // gNB SDAP is still not initiallized.

	ptrLTENR_SDAPVAR usdap = LTENR_UESDAP_GET(ueId, ueIf);
	if (!usdap)
		return; // UE SDAP is still not initiallized.

	if (isAssociated)
	{
	}
	else
	{
#pragma message(__LOC__"Implement dissociation for SDAP in function "__FUNCTION__)
	}
}
#pragma endregion

#pragma region SDAP_HDR
static void* LTENR_SDAP_HDR_CREATE(NETSIM_ID d, NETSIM_ID in, UINT * id)
{
	void* hdr = NULL;
	if (isUE(d, in))
	{
		*id = sdapULHDRId;
		hdr = calloc(1, sizeof(LTENR_SDAP_ULHDR));
	}
	else if (isGNB(d, in))
	{
		*id = sdapDLHDRId;
		hdr = calloc(1, sizeof(LTENR_SDAP_DLHDR));
	}
	else
	{
		fnNetSimError("LTENR-SDAP is called for device that is neither UE nor gNB.\n");
		*id = 0;
		hdr = NULL;
	}
	return hdr;
}
#pragma endregion

#pragma region SDAP_PACKETPROCESSING

static void LTENR_SDAP_FINDREMOTEDEVICE(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID * r, NETSIM_ID * rin,
	NETSIM_ID dest)
{
	*r = 0;
	*rin = 0;

	if (isUE(d, in))
	{
		*r = LTENR_FIND_ASSOCIATEDGNB(d, in);
		*rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, *r);
	}
	else
	{
		*rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, dest);
		if (*rin)* r = dest;
	}
	if (!*r)
		return;

	if (!fn_NetSim_LTENR_RRC_isConnected(d, in, *r, *rin))
	{
		*r = 0;
		*rin = 0;
	}
}

static void LTENR_SDAP_SETMODE(NETSIM_ID d, NETSIM_ID in,
	NetSim_PACKET * packet)
{
	LTENR_RLCMODE rlcMode;
	LTENR_LOGICALCHANNEL channel;
	switch (packet->nQOS)
	{
	case QOS_nrtPS:
	case QOS_ertPS:
	case QOS_rtPS:
	case QOS_UGS:
		rlcMode = LTENR_RLCMODE_UM;
		break;
	case QOS_BE:
	default:
		rlcMode = LTENR_RLCMODE_AM;
		break;
	}

	channel = LTENR_LOGICALCHANNEL_DTCH;

	LTENR_PACKET_SET_LOGICALCHANNEL(packet, channel);
	LTENR_PACKET_SET_RLCMODE(packet, rlcMode);
	print_ltenr_log("\tLogical Channel = %s\n"
		"\tRLC Mode = %s\n",
		strLTENR_LOGICALCHANNEL[channel], strLTENR_RLCMODE[rlcMode]);
}
void fn_NetSim_LTENR_SDAP_AddingQoSInfo() {

	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID r = packet->nReceiverId;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, r);
	NETSIM_IPAddress srcIP = packet->pstruNetworkData->szSourceIP;
	NETSIM_ID srcPort = packet->pstruTransportData->nSourcePort;
	NETSIM_ID desPort = packet->pstruTransportData->nDestinationPort;
	NETSIM_IPAddress desIP = packet->pstruNetworkData->szDestIP;

	ptrLTENR_SDAPENTITY e = LTENR_SDAP_FINDENTITY(d, in, r, rin, srcIP, srcPort, desIP, desPort);
	if (!e) {
		LTENR_SDAP_ESTABLISHENTITY(d, in, r, rin, srcIP, srcPort, desIP, desPort);
		LTENR_SDAP_ESTABLISHENTITY(r, rin, d, in, srcIP, srcPort, desIP, desPort);

		e = LTENR_SDAP_FINDENTITY(d, in, r, rin, srcIP, srcPort, desIP, desPort);
		e->appID = packet->pstruAppData->nApplicationId;
		e->countOut++;
		e->entityID = LTENR_SDAP_ENTITYCOUNT;
		LTENR_SDAP_ENTITYCOUNT++;
		e->QFI = LTENR_SDAP_QFICOUNT;
		LTENR_SDAP_QFICOUNT++;
		ptrLTENR_SDAP_DelayEntity Delaylist = LTENR_SDAP_DELAYENTITY_ALLOC();
		Delaylist->TxTime = pstruEventDetails->dEventTime;
		Delaylist->appID = packet->pstruAppData->nApplicationId;
		Delaylist->packetID = packet->nPacketId;
		Delaylist->segmentID = packet->pstruAppData->nSegmentId;
		LTENR_SDAP_DELAYENTITY_ADD(e, Delaylist);
		//fn_NetSim_LTE_SDAP_CreateFileDelay(e->entityID);
		//fn_NetSim_LTE_SDAP_CreateFilePER(e->entityID);
		//fn_NetSim_LTE_SDAP_CreateFilePDB(e->entityID);
		print_ltenr_log("Added QoS information for entity ID = %d\n", e->entityID);
	}
	else {
		e->countOut++;
		ptrLTENR_SDAP_DelayEntity Delaylist = LTENR_SDAP_DELAYENTITY_ALLOC();
		Delaylist->TxTime = pstruEventDetails->dEventTime;
		Delaylist->appID = packet->pstruAppData->nApplicationId;
		Delaylist->packetID = packet->nPacketId;
		Delaylist->segmentID = packet->pstruAppData->nSegmentId;
		LTENR_SDAP_DELAYENTITY_ADD(e, Delaylist);
		print_ltenr_log("Added QoS information for entity ID = %d\n", e->entityID);
	}

}

void fn_NetSim_LTENR_SDAP_HandleMacOut()
{
	LTENR_SDAP_PRINT_PACKET_LOG_OUT();
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;

	if (isBroadcastPacket(packet) ||
		isMulticastPacket(packet))
	{
		fnNetSimError("LTENR-Broadcast or multicast is not supported.\n");
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}



	NETSIM_ID r;
	NETSIM_ID rin;
	LTENR_SDAP_FINDREMOTEDEVICE(d, in, &r, &rin, get_first_dest_from_packet(packet));

	if (!r)
	{
		print_ltenr_log("No remote device is connected. Dropping packet\n");
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}
	packet->nReceiverId = r;
	packet->nTransmitterId = d;

	UINT id;
	void* hdr = LTENR_SDAP_HDR_CREATE(d, in, &id);
	if (!hdr)
	{
		print_ltenr_log("SDAP hdr creation failed. Dropping packet\n");
		fn_NetSim_Packet_FreePacket(packet);
		pstruEventDetails->pPacket = NULL;
		return;
	}

	print_ltenr_log("Adding SDAP header to packet. Hdr size = %d\n", LTENR_SDAP_HDR_LEN);
	LTENR_HDR_ADD_TO_PACKET(packet, hdr, id, LTENR_SDAP_HDR_LEN);
	LTENR_SDAP_SETMODE(d, in, packet);
	print_ltenr_log("Passing packet to PDCP\n");

	if (packet->nPacketType != PacketType_Control) {
		print_ltenr_log("Adding QoS information in crossponding entity\n");
		fn_NetSim_LTENR_SDAP_AddingQoSInfo();
	}

	LTENR_CallPDCPOut();
}

static void LTENR_SDAP_SENDTONETWORK(NETSIM_ID d, NETSIM_ID in,
	NetSim_PACKET * packet)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.dPacketSize = packet->pstruNetworkData->dPacketSize;
	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}
	pevent.nDeviceId = d;
	pevent.nDeviceType = DEVICE_TYPE(d);
	pevent.nEventType = NETWORK_IN_EVENT;
	pevent.nInterfaceId = in;
	pevent.nPacketId = packet->nPacketId;
	pevent.nProtocolId = DEVICE_NWPROTOCOL(d, in);
	pevent.pPacket = packet;
	fnpAddEvent(&pevent);
}

static void LTENR_SDAP_FORWARDTOOTHERINTERFACE(NETSIM_ID d, NETSIM_ID in,
	NetSim_PACKET * packet)
{
	bool isCopy = false;
	if (DEVICE(d)->nNumOfInterface > 2) isCopy = true;
	NetSim_PACKET* p = packet;
	NETSIM_ID i;
	for (i = 0; i < DEVICE(d)->nNumOfInterface; i++)
	{
		if (DEVICE_INTERFACE(d, i + 1)->nInterfaceId == in)
			continue;
		if (isCopy) p = fn_NetSim_Packet_CopyPacket(packet);
		NetSim_BUFFER * buf = DEVICE_ACCESSBUFFER(d, i + 1);
		if (!fn_NetSim_GetBufferStatus(buf))
		{
			NetSim_EVENTDETAILS pevent;
			memset(&pevent, 0, sizeof pevent);
			pevent.dEventTime = pstruEventDetails->dEventTime;
			pevent.dPacketSize = p->pstruNetworkData->dPacketSize;
			if (p->pstruAppData)
			{
				pevent.nApplicationId = p->pstruAppData->nApplicationId;
				pevent.nSegmentId = p->pstruAppData->nSegmentId;
			}
			pevent.nDeviceId = d;
			pevent.nDeviceType = DEVICE_TYPE(d);
			pevent.nEventType = MAC_OUT_EVENT;
			pevent.nInterfaceId = i + 1;
			pevent.nPacketId = p->nPacketId;
			pevent.nProtocolId = DEVICE_MACLAYER(d, i + 1)->nMacProtocolId;
			fnpAddEvent(&pevent);
		}
		fn_NetSim_Packet_AddPacketToList(buf, p, 0);
	}

	if (isCopy) fn_NetSim_Packet_FreePacket(packet);
}

void fn_NetSim_LTENR_SDAP_FileWriting(double data, UINT entityID, UINT Type) {
	FILE* fp = NULL;
	char filename[BUFSIZ], nodename[BUFSIZ];
	if (Type == 1) {
		sprintf(nodename, "plot_LTENR_SDAP_Delay_Entity%d.txt", entityID);
	}
	else if (Type == 2) {
		sprintf(nodename, "plot_LTENR_SDAP_PER_Entity%d.txt", entityID);
	}
	else if (Type == 3) {
		sprintf(nodename, "plot_LTENR_SDAP_PDB_Entity%d.txt", entityID);
	}
	sprintf(filename, "%s\\%s", pszIOPath, nodename);
	fp = fopen(filename, "a+");
	if (fp)
	{
		fprintf(fp, "\n%lf,%lf,", pstruEventDetails->dEventTime, data);
		fclose(fp);
	}

}
static ptrLTENR_SDAP_DelayEntity LTENR_SDAP_DELAY_FINDENTITY(ptrLTENR_SDAP_DelayEntity t) {
	while (t) {
		if (t->TxTime == pstruEventDetails->pPacket->pstruNetworkData->dArrivalTime)
			return t;
		t = LTENR_SDAP_DELAYENTITY_NEXT(t);
	}
	return NULL;
}

void fn_NetSim_LTENR_SDAP_CalcQOS() {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID r = packet->nTransmitterId;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, r);
	NETSIM_IPAddress srcIP = packet->pstruNetworkData->szSourceIP;
	NETSIM_ID srcPort = packet->pstruTransportData->nSourcePort;
	NETSIM_ID desPort = packet->pstruTransportData->nDestinationPort;
	NETSIM_IPAddress desIP = packet->pstruNetworkData->szDestIP;

	ptrLTENR_SDAPENTITY SDAPentity = LTENR_SDAP_FINDENTITY(r, rin, d, in, srcIP, srcPort, desIP, desPort);
	if (!SDAPentity) {
		print_ltenr_log("SDAP entity not found for packet id = %d",packet->nPacketId);
	}
	else {
		SDAPentity->countIn++;
		ptrLTENR_SDAP_DelayEntity Delaylist = LTENR_SDAP_DELAY_FINDENTITY(SDAPentity->delayList);
		if (Delaylist) {
			Delaylist->RxTime = pstruEventDetails->dEventTime;
#pragma warning (disable : 4244)
			Delaylist->delay = (double)abs(pstruEventDetails->dEventTime - Delaylist->TxTime);
#pragma warning (default : 4244)
			//mark packet as error if delay more than threshold
			if (Delaylist->delay > LTENR_SDAP_Transmission_Threshold)
				packet->nPacketStatus = PacketStatus_Error;

			if (packet->nQOS == QOS_UGS && Delaylist->TxTime + LTENR_SDAP_DefaultPDB <= Delaylist->RxTime)
				packet->nPacketStatus = PacketStatus_Error;
		}

		ptrLTENR_SDAP_DelayEntity t = SDAPentity->delayList;
		ptrLTENR_SDAP_DelayEntity t1 = NULL;
		double avgDelay = 0;
		UINT packetRx = 0;
		UINT packetTx = 0;
		double PDB = 0;
		if (pstruEventDetails->dEventTime > LTENR_SDAP_AVGWINDOW) {
			while (t) {
				//In packet
				if (t->RxTime >= pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW && t->delay <= LTENR_SDAP_Transmission_Threshold && t->RxTime >= t->TxTime) {
					avgDelay += t->delay;
					packetRx++;

					if (PDB < t->delay)
						PDB = t->delay;
				}

				//Out Packet
				if (t->TxTime <= LTENR_SDAP_AVGWINDOW)
					packetTx++;

				else if (t->TxTime >= pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW - LTENR_SDAP_Transmission_Threshold
					&& t->TxTime <= pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW && t->RxTime >= pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW
					&& t->delay <= LTENR_SDAP_Transmission_Threshold)
					packetTx++;

				else if (t->TxTime >= pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW 
					&& t->TxTime <= pstruEventDetails->dEventTime - LTENR_SDAP_Transmission_Threshold)
					packetTx++;

				else if (t->TxTime >= pstruEventDetails->dEventTime - LTENR_SDAP_Transmission_Threshold
					&& t->RxTime >= pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW && t->delay <= LTENR_SDAP_Transmission_Threshold)
					packetTx++;

				//delete entry which is not required to calc QOS
				if (t->RxTime < pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW && t->TxTime < pstruEventDetails->dEventTime - LTENR_SDAP_AVGWINDOW- LTENR_SDAP_Transmission_Threshold) {
					t1 = t;
				}

				t = LTENR_SDAP_DELAYENTITY_NEXT(t);

				if (t1) {
					LTENR_SDAP_DELAYENTITY_REMOVE(SDAPentity, t1);
					t1 = NULL;
				}
			}
			
				if (packetRx != 0)
					avgDelay = avgDelay / packetRx;
				//fn_NetSim_LTENR_SDAP_FileWriting(avgDelay, SDAPentity->entityID, 1);

				double PER = (double)(packetTx - packetRx) / packetTx;
				//fn_NetSim_LTENR_SDAP_FileWriting(PER, SDAPentity->entityID, 2);

				//fn_NetSim_LTENR_SDAP_FileWriting(PDB, SDAPentity->entityID, 3);

				print_ltenr_log("Writing QoS info for entity ID= %d\n", SDAPentity->entityID);
				print_ltenr_log("Calculated QoS in SDAP for entity ID= %d\n", SDAPentity->entityID);
		}
			
	}
}

void fn_NetSim_LTENR_SDAP_HandleMacIn()
{
	LTENR_SDAP_PRINT_PACKET_LOG_IN();
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;

	if (packet->nPacketType != PacketType_Control) {
		print_ltenr_log("Calculating QoS in SDAP for each entity\n");
		fn_NetSim_LTENR_SDAP_CalcQOS();
	}
	LTENR_MSG_FREE(packet);
	if (DEVICE_NWLAYER(d))
		LTENR_SDAP_SENDTONETWORK(d, in, packet);
	else
		LTENR_SDAP_FORWARDTOOTHERINTERFACE(d, in, packet);

	pstruEventDetails->pPacket = NULL;

}

int fn_NetSim_LTENR_SDAP_Metrics_F(PMETRICSWRITER metricsWriter)
{
	unsigned int i, j;

	PMETRICSNODE menu = init_metrics_node(MetricsNode_Menu, "LTENR_SDAP", NULL);
	PMETRICSNODE Plots_menu = init_metrics_node(MetricsNode_Menu, "Plots", NULL);
	PMETRICSNODE table = init_metrics_node(MetricsNode_Table, "LTENR_SDAP", NULL);
	PMETRICSNODE submenu = NULL;
	PMETRICSNODE submenu1 = NULL;

	add_table_heading(table, "AppID", true, 0);
	add_table_heading(table, "QFI", false, 0);
	add_table_heading(table, "SDAPEntityID", true, 0);
	add_table_heading(table, "SrcID", true, 0);
	add_table_heading(table, "DestID", true, 0);
	add_table_heading(table, "SrcIP:Port", false, 0);
	add_table_heading(table, "DestIP:Port", false, 0);
	add_table_heading(table, "PacketTx", true, 0);
	add_table_heading(table, "PacketRx", true, 0);
	//add_table_heading(table, "Delay", true, 0);
	//add_table_heading(table, "PER(Packet Error Rate)", true, 0);
	//add_table_heading(table, "PDB(Packet Delay Budget)", true, 0);
	

	for (i = 0; i < NETWORK->nDeviceCount; i++) {
		for (j = 0; j < NETWORK->ppstruDeviceList[i]->nNumOfInterface; j++) {
			if (!isLTE_NRInterface(i + 1, j + 1))
				continue;

			NETSIM_ID d = NETWORK->ppstruDeviceList[i]->nDeviceId;
			NETSIM_ID in = NETWORK->ppstruDeviceList[i]->ppstruInterfaceList[j]->nInterfaceId;
			ptrLTENR_SDAPVAR sdap = LTENR_SDAP_GET(d, in);
			ptrLTENR_SDAPENTITY e = sdap->sdapEntities;

			while (e) {
				if (e->entityID != 0) {
					char srcInfo[BUFSIZ], desInfo[BUFSIZ];
					sprintf(srcInfo, "%s:%d", e->srcIP->str_ip, e->srcPort);
					sprintf(desInfo, "%s:%d", e->desIP->str_ip, e->desPort);
					add_table_row_formatted(false, table, "%d,%d,%d,%d,%d,%s,%s,%d,%d,%d",
						e->appID, e->QFI, e->entityID, e->d, e->r, srcInfo, desInfo, e->countOut, e->countIn);
					/*
					//Delay
					char filename[BUFSIZ], nodename[BUFSIZ];
					sprintf(nodename, "LTENR_SDAP_Delay_Entity%d", e->entityID);
					sprintf(filename, "%s\\plot_%s.txt", pszIOPath, nodename);
					submenu1 = init_metrics_node(MetricsNode_Menu, nodename, NULL);
					add_node_to_menu(Plots_menu, submenu1);
					submenu = init_metrics_node(MetricsNode_Plot, nodename, filename);
					add_node_to_menu(submenu1, submenu);

					sprintf(nodename, "LTENR_SDAP_Delay_Entity%d", e->entityID);
					PMETRICSNODE g = init_metrics_node(MetricsNode_Link, "Delay_plot", nodename);
					add_table_row(true, table, g);


					//PER
					sprintf(nodename, "LTENR_SDAP_PER_Entity%d", e->entityID);
					sprintf(filename, "%s\\plot_%s.txt", pszIOPath, nodename);
					submenu1 = init_metrics_node(MetricsNode_Menu, nodename, NULL);
					add_node_to_menu(Plots_menu, submenu1);
					submenu = init_metrics_node(MetricsNode_Plot, nodename, filename);
					add_node_to_menu(submenu1, submenu);

					sprintf(nodename, "LTENR_SDAP_PER_Entity%d", e->entityID);
					PMETRICSNODE g1 = init_metrics_node(MetricsNode_Link, "PER_plot", nodename);
					add_table_row(true, table, g1);


					//PBD
					sprintf(nodename, "LTENR_SDAP_PDB_Entity%d", e->entityID);
					sprintf(filename, "%s\\plot_%s.txt", pszIOPath, nodename);
					submenu1 = init_metrics_node(MetricsNode_Menu, nodename, NULL);
					add_node_to_menu(Plots_menu, submenu1);
					submenu = init_metrics_node(MetricsNode_Plot, nodename, filename);
					add_node_to_menu(submenu1, submenu);

					sprintf(nodename, "LTENR_SDAP_PDB_Entity%d", e->entityID);
					PMETRICSNODE g2 = init_metrics_node(MetricsNode_Link, "PDB_plot", nodename);
					add_table_row(true, table, g2);
					*/

				}
				e = LTENR_SDAPNTITY_NEXT(e);
			}
		}
	}

	add_node_to_menu(menu, table);
	write_metrics_node(metricsWriter, WriterPosition_Current, NULL, menu);
	//write_metrics_node(metricsWriter, WriterPosition_Current, NULL, Plots_menu);


	return 1;
}
#pragma endregion
