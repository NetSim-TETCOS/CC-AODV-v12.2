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

#include "stdafx.h"
#include "LTENR_Msg.h"
#include "LTENR_RLC.h"
#include "LTENR_RLCMSG.h"

#pragma region FUNCTION_PROTOTYPE
//AMD HDR
void LTENR_RLCMAG_AMDHDR_Trace(ptrLTENR_RLCMSG_AMDHDR hdr, char* s);
void LTENR_RLCMSG_AMDHDR_FREE(ptrLTENR_RLCMSG_AMDHDR hdr);
ptrLTENR_RLCMSG_AMDHDR LTENR_RLCMSG_AMDHDR_COPY(ptrLTENR_RLCMSG_AMDHDR hdr);

//UMD HDR
void LTENR_RLCMSG_UMDHDR_Trace(ptrLTENR_RLCMSG_UMDHDR hdr, char* s);
void LTENR_RLCMSG_UMDHDR_FREE(ptrLTENR_RLCMSG_UMDHDR hdr);
ptrLTENR_RLCMSG_UMDHDR LTENR_RLCMSG_UMDHDR_COPY(ptrLTENR_RLCMSG_UMDHDR hdr);

//STATUS PDU
ptrLTENR_RLCMSG_STATUSPDU LTENR_RLCMSG_STATUSPDU_COPY(ptrLTENR_RLCMSG_STATUSPDU hdr);
void LTENR_RLCMAG_STATUSPDU_FREE(ptrLTENR_RLCMSG_STATUSPDU status);
void LTENR_RLCMAG_STATUSPDU_Trace(ptrLTENR_RLCMSG_STATUSPDU hdr, char* s);

#pragma endregion

#pragma region RLCMSG_INIT
void LTENR_RLCMSG_INIT()
{
	rlcMsgTMDHDRID = LTENR_HDR_REGISTER("RLC_TMD_HDR",
										NULL, NULL, NULL,NULL);
	rlcMsgUMDHDRID = LTENR_HDR_REGISTER("RLC_UMD_HDR",
										NULL, LTENR_RLCMSG_UMDHDR_COPY, LTENR_RLCMSG_UMDHDR_FREE, LTENR_RLCMSG_UMDHDR_Trace);
	rlcMsgAMDHDRID = LTENR_HDR_REGISTER("RLC_AMD_HDR",
										NULL, LTENR_RLCMSG_AMDHDR_COPY, LTENR_RLCMSG_AMDHDR_FREE, LTENR_RLCMAG_AMDHDR_Trace);
	rlcMsgSTATUSPDUHDRID = LTENR_HDR_REGISTER("RLC_STATUS_PDU_HDR",
											  NULL, LTENR_RLCMSG_STATUSPDU_COPY, LTENR_RLCMAG_STATUSPDU_FREE, LTENR_RLCMAG_STATUSPDU_Trace);
}
#pragma endregion

#pragma region RLCMSG_TMDHDR
void LTENR_RLCMSG_TMHDR_ADD(NetSim_PACKET* packet)
{
	return;
}
#pragma endregion

#pragma region RLCMSG_UMDHDR
UINT LTENR_RLCMSG_UMHDR_ADD(NetSim_PACKET* packet, SI si, UINT sn, UINT16 so)
{
	UINT size = LTENR_RLCMSG_UMDHDR_SIZE(si);

	ptrLTENR_RLCMSG_UMDHDR hdr = calloc(1, sizeof* hdr);
	LTENR_HDR_ADD_TO_PACKET(packet, hdr, rlcMsgUMDHDRID, size);
	hdr->si = si;
	hdr->SN = sn;
	hdr->SO = so;
	return size;
}

ptrLTENR_RLCMSG_UMDHDR LTENR_RLCMSG_UMHDR_GET(NetSim_PACKET* packet)
{
	return (ptrLTENR_RLCMSG_UMDHDR)LTENR_HDR_GET_FROM_PACKET(packet, rlcMsgUMDHDRID);
}

void LTENR_RLCMSG_UMDHDR_REMOVE(NetSim_PACKET* packet)
{
	LTENR_HDR_FREE_FROM_PACKET(packet, rlcMsgUMDHDRID);
}

ptrLTENR_RLCMSG_UMDHDR LTENR_RLCMSG_UMDHDR_COPY(ptrLTENR_RLCMSG_UMDHDR hdr)
{
	ptrLTENR_RLCMSG_UMDHDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	return ret;
}

void LTENR_RLCMSG_UMDHDR_FREE(ptrLTENR_RLCMSG_UMDHDR hdr)
{
	free(hdr);
}

void LTENR_RLCMSG_UMDHDR_Trace(ptrLTENR_RLCMSG_UMDHDR hdr, char* s)
{
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"SI = %s\nSN=%d\nSO=%d\nPollBit=%d\n",
		"RLC_UMD_HDR", strSI[hdr->si], hdr->SN, hdr->SO, hdr->R);
	strcat(s, info);
}
#pragma endregion

#pragma region RLCMSG_AMDHDR
UINT LTENR_RLCMSG_AMHDR_ADD(NetSim_PACKET* packet, SI si, UINT sn, UINT16 so)
{
	UINT size = LTENR_RLCMSG_AMDHDR_SIZE(si);

	ptrLTENR_RLCMSG_AMDHDR hdr = calloc(1, sizeof* hdr);
	LTENR_HDR_ADD_TO_PACKET(packet, hdr, rlcMsgAMDHDRID, size);
	hdr->si = si;
	hdr->SN = sn;
	hdr->SO = so;
	return size;
}

ptrLTENR_RLCMSG_AMDHDR LTENR_RLCMSG_AMHDR_GET(NetSim_PACKET* packet)
{
	return (ptrLTENR_RLCMSG_AMDHDR)LTENR_HDR_GET_FROM_PACKET(packet, rlcMsgAMDHDRID);
}

void LTENR_RLCMSG_AMDHDR_REMOVE(NetSim_PACKET* packet)
{
	LTENR_HDR_FREE_FROM_PACKET(packet, rlcMsgAMDHDRID);
}

void LTENR_RLCMSG_AMDHDR_SETPOLL(NetSim_PACKET* packet)
{
	LTENR_RLCMSG_AMHDR_GET(packet)->P = 1;
}

ptrLTENR_RLCMSG_AMDHDR LTENR_RLCMSG_AMDHDR_COPY(ptrLTENR_RLCMSG_AMDHDR hdr)
{
	ptrLTENR_RLCMSG_AMDHDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	return ret;
}

void LTENR_RLCMSG_AMDHDR_FREE(ptrLTENR_RLCMSG_AMDHDR hdr)
{
	free(hdr);
}

void LTENR_RLCMAG_AMDHDR_Trace(ptrLTENR_RLCMSG_AMDHDR hdr, char* s)
{
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
			"SI = %s\nSN=%d\nSO=%d\nPollBit=%d\n",
			"RLC_AMD_HDR", strSI[hdr->si], hdr->SN, hdr->SO, hdr->P);
	strcat(s, info);
}
#pragma endregion

#pragma region RLCMSG_STATUSPDU
ptrLTENR_RLCMSG_STATUSPDU LTENR_RLCMSG_STATUSPDU_NEW()
{
	ptrLTENR_RLCMSG_STATUSPDU s = calloc(1, sizeof * s);
	s->dcBit = DC_CONTROL;
	return s;
}

NetSim_PACKET* LTENR_RLCMSG_STATUSPDU_AddToPacket(void* ent, ptrLTENR_RLCMSG_STATUSPDU status, UINT size)
{
	ptrLTENR_RLCENTITY entity = (ptrLTENR_RLCENTITY)ent;
	NetSim_PACKET* p = LTENR_PACKET_CREATE(entity->d, entity->peerId,
		pstruEventDetails->dEventTime, LTENR_MSG_RLC_STATUSPDU);
	LTENR_PACKET_SET_RLCMODE(p, LTENR_RLCMODE_TM);
	LTENR_PACKET_SET_LOGICALCHANNEL(p, LTENR_LOGICALCHANNEL_CCCH);
	LTENR_HDR_ADD_TO_PACKET(p, status, rlcMsgSTATUSPDUHDRID, size + LTENR_RLCMSG_STATUSPDU_FIX_LEN);
	return p;
}

void LTENR_RLCMAG_STATUSPDU_FREE(ptrLTENR_RLCMSG_STATUSPDU status)
{
	ptrLTENR_RLCMSG_STATUSPDU_NACK nack = status->nackHead;
	while (nack)
	{
		ptrLTENR_RLCMSG_STATUSPDU_NACK f = nack;
		nack = nack->next;
		free(f);
	}
	free(status);
}

ptrLTENR_RLCMSG_STATUSPDU LTENR_RLCMSG_STATUSPDU_COPY(ptrLTENR_RLCMSG_STATUSPDU hdr)
{
	ptrLTENR_RLCMSG_STATUSPDU ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	return ret;
}

void LTENR_RLCMAG_STATUSPDU_Trace(ptrLTENR_RLCMSG_STATUSPDU hdr, char* s)
{
	char info[BUFSIZ];
	if (hdr->E1) {
		sprintf(info, "Hdr Type = %s\n"
			"CPT = %d\nSdcBit=%d\nACK_SN=%d\nNACKCount=%d\n",
			"STATUS_PDU_HDR", hdr->CPT, hdr->dcBit, hdr->Ack_SN, hdr->NACKCount);
	}
	else {
		sprintf(info, "Hdr Type = %s\n"
			"CPT = %d\nSdcBit=%d\nACK_SN=%d\n",
			"STATUS_PDU_HDR", hdr->CPT, hdr->dcBit, hdr->Ack_SN);
	}
	strcat(s, info);
}

void LTENR_RLCMSG_STATUSPDU_AddNack(ptrLTENR_RLCMSG_STATUSPDU status, ptrLTENR_RLCMSG_STATUSPDU_NACK nack)
{
	if (status->nackHead)
	{
		status->nackTail->next = nack;
		status->nackTail = nack;
	}
	else
	{
		status->nackHead = nack;
		status->nackTail = nack;
	}
	status->NACKCount++;
}

ptrLTENR_RLCMSG_STATUSPDU LTENR_RLCMSG_STATUSPDU_GET(NetSim_PACKET* packet)
{
	return (ptrLTENR_RLCMSG_STATUSPDU)LTENR_HDR_GET_FROM_PACKET(packet, rlcMsgSTATUSPDUHDRID);
}

bool LTENR_RLCAM_STATUSPDU_FindSN(ptrLTENR_RLCMSG_STATUSPDU pdu, UINT sn)
{
	ptrLTENR_RLCMSG_STATUSPDU_NACK nack = pdu->nackHead;
	while (nack)
	{
		if (nack->NAck_SN == sn)
			return true;
		nack = nack->next;
	}
	return false;
}

#pragma endregion

void LTENR_RLC_SENDPDUTOMAC(NETSIM_ID d, NETSIM_ID in,
							NetSim_PACKET* packet)
{
#ifdef LTENR_RLC_TRANSMISSIONSTATUSNOTIFICATION_LOG
	print_ltenr_log("Sending packet to MAC sublayer\n");
	LTENR_RLCMSG_PRINT(packet);
#endif
	
	NetSim_EVENTDETAILS pevent;
	memcpy(&pevent, pstruEventDetails, sizeof pevent);

	memset(pstruEventDetails, 0, sizeof* pstruEventDetails);
	pstruEventDetails->dEventTime = pevent.dEventTime;
	pstruEventDetails->dPacketSize = packet->pstruMacData->dPacketSize;
	if (packet->pstruAppData)
	{
		pstruEventDetails->nApplicationId = packet->pstruAppData->nApplicationId;
		pstruEventDetails->nSegmentId = packet->pstruAppData->nSegmentId;
	}
	pstruEventDetails->nDeviceId = d;
	pstruEventDetails->nDeviceType = DEVICE_TYPE(d);
	pstruEventDetails->nEventType = MAC_OUT_EVENT;
	pstruEventDetails->nInterfaceId = in;
	pstruEventDetails->nPacketId = packet->nPacketId;
	pstruEventDetails->nPrevEventId = pevent.nEventId;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(d, in);
	if (pd->series3GPP == 38)
		pstruEventDetails->nProtocolId = MAC_PROTOCOL_LTE_NR;
	else
		pstruEventDetails->nProtocolId = MAC_PROTOCOL_LTE;
	pstruEventDetails->pPacket = packet;
	LTENR_CallMACOut();

	memcpy(pstruEventDetails, &pevent, sizeof* pstruEventDetails);
}

void LTENR_RLCMSG_PRINT(NetSim_PACKET* packet)
{
	char type[50];
	print_ltenr_log("\tPacket Id = %d\n"
					"\tPacket Type = %s\n"
					"\tSource Id = %d\n"
					"\tDestination Id = %d\n"
					"\tPacket size = %d\n"
					"\tMAC payload size = %d\n",
					packet->nPacketId,
					fn_NetSim_Config_GetControlPacketType(packet, type),
					packet->nSourceId,
					get_first_dest_from_packet(packet),
					(UINT)packet->pstruMacData->dPacketSize,
					(UINT)packet->pstruMacData->dPayload);

	LTENR_RLCMODE mode = LTENR_PACKET_GET_RLCMODE(packet);
	switch (mode)
	{
		case LTENR_RLCMODE_TM:
			print_ltenr_log("\tRLC hdr type = TMD Hdr\n");
			print_ltenr_log("\tHdr size = 0\n");
			break;
		case LTENR_RLCMODE_UM:
		{
			ptrLTENR_RLCMSG_UMDHDR uhdr = LTENR_HDR_GET_FROM_PACKET(packet, rlcMsgUMDHDRID);
			print_ltenr_log("\tRLC hdr type = UMD Hdr\n");
			print_ltenr_log("\tHdr size = %d\n"
							"\tSI = %s\n",
							LTENR_RLCMSG_UMDHDR_SIZE(uhdr->si),
							strSI[uhdr->si]);
			if(uhdr->SN)
				print_ltenr_log("\tSN = %d\n", uhdr->SN);
			if (uhdr->SO)
				print_ltenr_log("\tSO = %d\n", uhdr->SO);
		}
		break;
		case LTENR_RLCMODE_AM:
		{
			ptrLTENR_RLCMSG_AMDHDR ahdr = LTENR_HDR_GET_FROM_PACKET(packet, rlcMsgAMDHDRID);
			print_ltenr_log("\tRLC hdr type = AMD Hdr\n");
			print_ltenr_log("\tHdr size = %d\n"
							"\tSI = %s\n"
							"\tSN = %d\n"
							"\tSO = %d\n"
							"\tPolling Bit = %d\n",
							LTENR_RLCMSG_AMDHDR_SIZE(ahdr->si),
							strSI[ahdr->si], ahdr->SN, ahdr->SO, ahdr->P);
		}
		break;
	}
}
