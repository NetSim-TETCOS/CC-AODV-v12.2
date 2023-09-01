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
* Author:    Kumar Gaurav	                                                        *
*										                                            *
* ----------------------------------------------------------------------------------*/

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_GNBRRC.h"
#include "LTENR_NAS.h"
#include "LTENR_PHY.h"
#include "LTENR_EPC.h"
#pragma endregion

#pragma region ADD_PACKET_INTO_BUFFER
void fn_NetSim_NAS_ADD_PACKET_INTO_BUUFER(void* msg) {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	bool isCopy = false;
	if (DEVICE(d)->nNumOfInterface > 2) isCopy = true;
	NetSim_PACKET* p = pstruEventDetails->pPacket;
	NETSIM_ID i;
	for (i = 0; i < DEVICE(d)->nNumOfInterface; i++)
	{
		if (DEVICE_INTERFACE(d, i + 1)->nInterfaceId == in)
			continue;
		if (isCopy) p = fn_NetSim_Packet_CopyPacket(pstruEventDetails->pPacket);
		NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(d, i + 1);
		if (!fn_NetSim_GetBufferStatus(buf))
		{
			NetSim_EVENTDETAILS pevent;
			memset(&pevent, 0, sizeof pevent);
			pevent.dEventTime = pstruEventDetails->dEventTime;
			pevent.dPacketSize = sizeof(msg);
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

	if (isCopy) fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
}
#pragma endregion

#pragma region HANDOVER_COMPLETE
void fn_NetSim_LTENR_NAS_HANDOVER_COMPLETE(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->serveringCellID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->serveringCellID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = hdr->serveringCellIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->serveringCellID, hdr->serveringCellIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_HANDOVER_COMPLETE);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->serveringCellID, hdr->targetCellID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_COMPLETE);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, nasHandoverComplete, LTENR_MSG_NAS_HANDOVER_COMPLETE);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_COMPLETE() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasHandoverComplete);
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_COMPLETE;
	LTENR_CallPDCPOut();
}
void fn_NetSim_LTENR_NAS_HANDOVER_COMPLETE_RECV() {
	//do nothing 
}
#pragma endregion

#pragma region UE_CONTEXT_RELEASE_ACK
void fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE_ACK(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->serveringCellID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->serveringCellID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = hdr->serveringCellIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->serveringCellID, hdr->serveringCellIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_UECONTEXTRELEASE_ACK);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->serveringCellID, hdr->targetCellID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, nasUEContextReleaseACK, LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_UE_CONTEXT_RELEASE_ACK() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasUEContextReleaseACK);
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK;
	ptrLTENR_NAS_UECONTEXTRELEASECOMMAND msg = calloc(1, sizeof * msg);
	msg->ueID = hdr->UEID;
	msg->outCome = HANDOVER_OUTCOME_SUCESSFUL;
	msg->cause = HANDOVER_CAUSE_RADIO_NETWORK;
	hdr->msg = msg;
	pstruEventDetails->pPacket->pstruMacData->dontFree = true;
	fn_NetSim_NAS_ADD_PACKET_INTO_BUUFER(msg);
}
void fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE_ACK_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_UECONTEXTRELEASECOMMAND msg = hdr->msg;
	if (msg->outCome != HANDOVER_OUTCOME_SUCESSFUL) {
		fnNetSimError("UE context release outcome is unsucessfull.");
	}
}
#pragma endregion

#pragma region UE_CONTEXT_RELEASE
void fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->targetCellID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->targetCellID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = hdr->targetCellIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->targetCellID, hdr->targetCellIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_UECONTEXTRELEASE);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->targetCellID, hdr->serveringCellID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, nasUEContextRelease, LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_UE_CONTEXT_RELEASE() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasUEContextRelease);
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE;
	ptrLTENR_NAS_UECONTEXTRELEASEREQUEST msg = calloc(1, sizeof * msg);
	msg->cause = HANDOVER_CAUSE_RADIO_NETWORK;
	msg->gnb_UE_ID = hdr->targetCellIF;
	msg->mme_UE_ID = hdr->AMFID;
	hdr->msg = msg;
	pstruEventDetails->pPacket->pstruMacData->dontFree = true;
	fn_NetSim_NAS_ADD_PACKET_INTO_BUUFER(msg);
}
void fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE_ACK(hdr);
}
#pragma endregion

#pragma region PATH_SWITCH_ACK
void fn_NetSim_LTENR_NAS_PATH_SWITCH_ACK(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->AMFID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->AMFID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = LTENR_EPC_HLR_FindOutInterface(hdr->AMFID, hdr->UEID);
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->AMFID, hdr->AMFIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_PATH_SWITCH_ACK);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->AMFID, hdr->targetCellID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, naspathSwitchAck, LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_PATH_SWITCH_REQUEST_ACK() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, naspathSwitchAck);
	ptrLTENR_NAS_PATH_SWITCH_REQUEST msg = calloc(1, sizeof * msg);
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK;
	//path switch
	//NETSIM_ID epcIn = LTENR_EPC_HLR_FindOutInterface(hdr->AMFID, hdr->serveringCellID);
	//ptrLTENR_EPC_HLR hlr = LTENR_EPC_FindHLR(hdr->AMFID, epcIn, hdr->serveringCellID);
	//ptrLTENR_EPC epc = LTENR_EPC_GET(hdr->AMFID, epcIn);
	//LTENR_EPC_HLR_REMOVE(epc, hlr);
	pstruEventDetails->pPacket->pstruMacData->dontFree = true;
	//add into buffer
	NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	NetSim_PACKET* p = pstruEventDetails->pPacket;
	if (!fn_NetSim_GetBufferStatus(buf))
	{
		NetSim_EVENTDETAILS pevent;
		memset(&pevent, 0, sizeof pevent);
		pevent.dEventTime = pstruEventDetails->dEventTime;
		pevent.dPacketSize = sizeof(msg);
		pevent.nDeviceId = pstruEventDetails->nDeviceId;
		pevent.nDeviceType = DEVICE_TYPE(pstruEventDetails->nDeviceId);
		pevent.nEventType = MAC_OUT_EVENT;
		pevent.nInterfaceId = pstruEventDetails->nInterfaceId;
		pevent.nPacketId = p->nPacketId;
		pevent.nProtocolId = DEVICE_MACLAYER(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId)->nMacProtocolId;
		fnpAddEvent(&pevent);
	}
	fn_NetSim_Packet_AddPacketToList(buf, p, 0);
	pstruEventDetails->pPacket = NULL;
}
void fn_NetSim_LTENR_NAS_PATH_SWITCH_ACK_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE(hdr);
}
#pragma endregion

#pragma region PATH_SWITCH_REQUEST
void fn_NetSim_LTENR_NAS_PATH_SWITCH_REQUEST(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->targetCellID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->targetCellID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = hdr->targetCellIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->targetCellID, hdr->targetCellIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_PATH_SWITCH);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->targetCellID, hdr->AMFID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_PATH_SWITCH);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, nasPathSwitch, LTENR_MSG_NAS_HANDOVER_PATH_SWITCH);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_PATH_SWITCH_REQUEST() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasPathSwitch);
	ptrLTENR_NAS_PATH_SWITCH_REQUEST msg = calloc(1, sizeof * msg);
	msg->targetGNBID = hdr->targetCellID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_PATH_SWITCH;
	pstruEventDetails->pPacket->pstruMacData->dontFree = true;
	//add into buffer
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = fn_NetSim_OTHER_INTERFACE_ID(d, pstruEventDetails->nInterfaceId);
	NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(d, in);
	NetSim_PACKET* p = pstruEventDetails->pPacket;
	if (!fn_NetSim_GetBufferStatus(buf))
	{
		NetSim_EVENTDETAILS pevent;
		memset(&pevent, 0, sizeof pevent);
		pevent.dEventTime = pstruEventDetails->dEventTime;
		pevent.dPacketSize = sizeof(msg);
		pevent.nDeviceId = pstruEventDetails->nDeviceId;
		pevent.nDeviceType = DEVICE_TYPE(d);
		pevent.nEventType = MAC_OUT_EVENT;
		pevent.nInterfaceId = in;
		pevent.nPacketId = p->nPacketId;
		pevent.nProtocolId = DEVICE_MACLAYER(d, in)->nMacProtocolId;
		fnpAddEvent(&pevent);
	}
	fn_NetSim_Packet_AddPacketToList(buf, p, 0);
	pstruEventDetails->pPacket = NULL;
}
void fn_NetSim_LTENR_NAS_PATH_SWITCH_REQUEST_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	pstruEventDetails->pPacket = NULL;
	fn_NetSim_LTENR_NAS_PATH_SWITCH_ACK(hdr);
}
#pragma endregion

#pragma region BUFFER_TRANSFER
void fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(ptrLTENR_PDCPENTITY ser, ptrLTENR_PDCPENTITY tar, int flag) {
	if (!ser)
		return;
	if (!tar)
		return;

	tar->transmissionBuffer = ser->transmissionBuffer;
	if (tar->transmissionBuffer->headPacket != NULL) {
		ptrLTENR_BUFFEREDPACKET buf = tar->transmissionBuffer->headPacket;
		NetSim_PACKET* head = tar->transmissionBuffer->headPacket->packet;
		NetSim_PACKET* packet = NULL;
		while (buf) {
			packet = buf->packet;
			while (packet) {
				if (flag == 0)
					packet->nTransmitterId = tar->enbId;
				else
					packet->nReceiverId = tar->enbId;
				packet = packet->pstruNextPacket;
			}
			buf = buf->next;
		}
		tar->transmissionBuffer->headPacket->packet = head;
	}
	
	ser->transmissionBuffer = NULL;
	tar->receiveBuffer = ser->receiveBuffer;
	ser->receiveBuffer = NULL;
	tar->TX_NEXT = ser->TX_NEXT;
	tar->last_RECV_COUNT = ser->last_RECV_COUNT;
	tar->RX_DELIV = ser->RX_DELIV;
	tar->RX_NEXT = ser->RX_NEXT;
	tar->RX_REORD = ser->RX_REORD;
}
void fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(ptrLTENR_RLCENTITY ser, ptrLTENR_RLCENTITY tar, int flag) {
	if (!ser)
		return;
	if (!tar)
		return;

	tar->transmissionBuffer = ser->transmissionBuffer;
	ser->transmissionBuffer = NULL;
	tar->AM = ser->AM;
	if (tar->AM.receptionBuffer != NULL) {
		ptrLTENR_RLC_RECEPTIONBUFFER buf = tar->AM.receptionBuffer;
		NetSim_PACKET* head = tar->AM.receptionBuffer->head;
		NetSim_PACKET* packet = NULL;
		while (buf) {
			packet = buf->head;
			while (packet) {
				if (flag == 0)
					packet->nReceiverId = tar->d;
				else
					packet->nTransmitterId = tar->peerId;
				packet = packet->pstruNextPacket;
			}
			buf = LTENR_RLC_RECEPTIONBUFFER_NEXT(buf);
		}
		tar->AM.receptionBuffer->head = head;
	}
	ser->AM.receptionBuffer = NULL;
	tar->UM = ser->UM;
	if (tar->UM.receptionBuffer != NULL) {
		ptrLTENR_RLC_RECEPTIONBUFFER buf = tar->UM.receptionBuffer;
		NetSim_PACKET* head = tar->UM.receptionBuffer->head;
		NetSim_PACKET* packet = NULL;
		while (buf) {
			packet = buf->head;
			while (packet) {
				if (flag == 0)
					packet->nReceiverId = tar->d;
				else
					packet->nTransmitterId = tar->peerId;
				packet = packet->pstruNextPacket;
			}
			buf = LTENR_RLC_RECEPTIONBUFFER_NEXT(buf);
		}
		tar->UM.receptionBuffer->head = head;
	}
	ser->UM.receptionBuffer = NULL;
	tar->retransmissionBuffer = ser->retransmissionBuffer;
	ser->retransmissionBuffer = NULL;
	tar->rlcMode = ser->rlcMode;
	tar->transmittedBuffer = ser->transmittedBuffer;
	tar->pollRetransmitTimerId = ser->pollRetransmitTimerId;
	tar->isPollRetransmitTimerStarted = ser->isPollRetransmitTimerStarted;	
	tar->reassemblyTimerId = ser->reassemblyTimerId;
	tar->isReassemblyTimerStarted = ser->isReassemblyTimerStarted;
	if (ser->isReassemblyTimerStarted) {
		fnDeleteEvent(ser->reassemblyTimerId);
		LTENR_RLCAM_Start_t_ReassemblyTimer(tar);
	}
	tar->statusProhibitTimerId = ser->statusProhibitTimerId;
	tar->isStatusProhibitTimerStarted = ser->isStatusProhibitTimerStarted;
	ser->transmittedBuffer = NULL;
	for (int i = 0; i < LTENR_LOGICALCHANNEL_LAST; i++)
		tar->bufferStatus[i] = ser->bufferStatus[i];
}
void fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_TRANSMISSION_ID(ptrLTENR_RLCENTITY entity, NETSIM_ID t) {
	if (!entity)
		return;

	ptrLTENR_RLCBUFFER buf = entity->transmissionBuffer;
	while (buf)
	{
		buf->packet->nTransmitterId = t;
		LTENR_RLCBUFFER_NEXT(buf);
	}

	ptrLTENR_RLCBUFFER buf1 = entity->retransmissionBuffer;
	while (buf1)
	{
		buf1->packet->nTransmitterId = t;
		LTENR_RLCBUFFER_NEXT(buf1);
	}

}

void fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_RECEIVER_ID(ptrLTENR_RLCENTITY entity, NETSIM_ID t) {
	if (!entity)
		return;

	ptrLTENR_RLCBUFFER buf = entity->transmissionBuffer;
	while (buf)
	{
		buf->packet->nReceiverId = t;
		LTENR_RLCBUFFER_NEXT(buf);
	}

	ptrLTENR_RLCBUFFER buf1 = entity->retransmissionBuffer;
	while (buf1)
	{
		buf1->packet->nReceiverId = t;
		LTENR_RLCBUFFER_NEXT(buf1);
	}

}

void fn_NetSim_LTENR_NAS_RLC_UE_BUFFER_TRANSFER(NETSIM_ID d, NETSIM_ID in,
													NETSIM_ID r, NETSIM_ID rin,
													NETSIM_ID t, NETSIM_ID tin) {

	ptrLTENR_RLCENTITY SerEntity = LTENR_RLC_FINDENTITY(r, rin, d, in, LTENR_RLCMODE_TM, true, false);
	ptrLTENR_RLCENTITY TarEntity = LTENR_RLC_FINDENTITY(r, rin, t, tin, LTENR_RLCMODE_TM, true, false);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 1);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_RECEIVER_ID(TarEntity, t);

	SerEntity = LTENR_RLC_FINDENTITY(r, rin, d, in, LTENR_RLCMODE_TM, false, true);
	TarEntity = LTENR_RLC_FINDENTITY(r, rin, t, tin, LTENR_RLCMODE_TM, false, true);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 1);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_RECEIVER_ID(TarEntity, t);

	SerEntity = LTENR_RLC_FINDENTITY(r, rin, d, in, LTENR_RLCMODE_UM, true, false);
	TarEntity = LTENR_RLC_FINDENTITY(r, rin, t, tin, LTENR_RLCMODE_UM, true, false);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 1);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_RECEIVER_ID(TarEntity, t);

	SerEntity = LTENR_RLC_FINDENTITY(r, rin, d, in, LTENR_RLCMODE_UM, false, true);
	TarEntity = LTENR_RLC_FINDENTITY(r, rin, t, tin, LTENR_RLCMODE_UM, false, true);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 1);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_RECEIVER_ID(TarEntity, t);

	SerEntity = LTENR_RLC_FINDENTITY(r, rin, d, in, LTENR_RLCMODE_AM, true, true);
	TarEntity = LTENR_RLC_FINDENTITY(r, rin, t, tin, LTENR_RLCMODE_AM, true, true);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 1);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_RECEIVER_ID(TarEntity, t);

}
void fn_NetSim_LTENR_NAS_RLC_GNB_BUFFER_TRANSFER(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin,
	NETSIM_ID t, NETSIM_ID tin) {

	ptrLTENR_RLCENTITY SerEntity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_TM, true, false);
	ptrLTENR_RLCENTITY TarEntity = LTENR_RLC_FINDENTITY(t, tin, r, rin, LTENR_RLCMODE_TM, true, false);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 0);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_TRANSMISSION_ID(TarEntity, t);
	SerEntity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_TM, false, true);
	TarEntity = LTENR_RLC_FINDENTITY(t, tin, r, rin, LTENR_RLCMODE_TM, false, true);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 0);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_TRANSMISSION_ID(TarEntity, t);
	SerEntity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_UM, true, false);
	TarEntity = LTENR_RLC_FINDENTITY(t, tin, r, rin, LTENR_RLCMODE_UM, true, false);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 0);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_TRANSMISSION_ID(TarEntity, t);
	SerEntity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_UM, false, true);
	TarEntity = LTENR_RLC_FINDENTITY(t, tin, r, rin, LTENR_RLCMODE_UM, false, true);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 0);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_TRANSMISSION_ID(TarEntity, t);
	SerEntity = LTENR_RLC_FINDENTITY(d, in, r, rin, LTENR_RLCMODE_AM, true, true);
	TarEntity = LTENR_RLC_FINDENTITY(t, tin, r, rin, LTENR_RLCMODE_AM, true, true);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER(SerEntity, TarEntity, 0);
	fn_NetSim_LTENR_NAS_RLC_BUFFER_TRANSFER_CHANGE_TRANSMISSION_ID(TarEntity, t);
}
#pragma endregion

#pragma region HANDOVER_COMMAND
void fn_NetSim_LTENR_NAS_HANDOVER_COMMAND(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->serveringCellID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->serveringCellID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = hdr->serveringCellIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->serveringCellID, hdr->serveringCellIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_HANDOVER_COMMAND);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->serveringCellID, hdr->UEID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_COMMAND);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, nasHandoverTrigger, LTENR_MSG_NAS_HANDOVER_COMMAND);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_COMMAND() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasHandoverTrigger);
	ptrLTENR_NAS_HANDOVER_COMMAND msg = calloc(1, sizeof * msg);
	msg->E_RADataForwading_DL_TransportLayerAddress = DEVICE_NWADDRESS(hdr->targetCellID, hdr->targetCellIF);
	msg->MME_UE_ID = hdr->AMFID;
	msg->type = HANDOVER_TYPE_INTRA_LTENR;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_COMMAND;
	pstruEventDetails->pPacket->pstruMacData->dontFree = true;
	LTENR_CallPDCPOut();
}

void fn_NetSim_LTENR_NAS_HANDOVER_COMMAND_HANDLE(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = pstruEventDetails->nDeviceId;
	pevent.nDeviceType = DEVICE_TYPE(pstruEventDetails->nDeviceId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = pstruEventDetails->nInterfaceId;
	pevent.szOtherDetails = hdr;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_HANDOVER_COMMAND_HANDLE);
	fnpAddEvent(&pevent);
}

void fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_COMMAND_HANDLE() {
	ptrLTENR_HANDOVER_Hdr hdr = pstruEventDetails->szOtherDetails;
	NETSIM_ID d = hdr->serveringCellID;
	NETSIM_ID in = hdr->serveringCellIF;
	NETSIM_ID r = hdr->UEID;
	NETSIM_ID rin = hdr->UEIF;
	NETSIM_ID t = hdr->targetCellID;
	NETSIM_ID tin = hdr->targetCellIF;

	NETSIM_ID epcIn = LTENR_EPC_HLR_FindOutInterface(hdr->AMFID, hdr->UEID);

	//Association and Dissociation of interface
	LTENR_ASSOCIATEINFO_REMOVE(d, in, r, rin);
	LTENR_ASSOCIATEINFO_ADD(t, tin, r, rin);

	LTENR_PHY_ASSOCIATION(t, tin, r, rin, true);
	LTENR_PDCP_ASSOCIATION(t, tin, r, rin, true);
	LTENR_RLC_ASSOCIATION(t, tin, r, rin, true);
	ptrLTENR_UERRC ueRRC = LTENR_UERRC_GET(r, rin);

	ptrLTENR_PDCPVAR serPDCPVar = LTENR_GNBPDCP_GET(d, in);
	ptrLTENR_PDCPVAR tarPDCPVar = LTENR_GNBPDCP_GET(t, tin);
	ptrLTENR_PDCPVAR uePDCPVar = LTENR_UEPDCP_GET(r, rin);

	//Buffer transfer GNB PDCP 
	ptrLTENR_PDCPENTITY serentity = LTENR_PDCP_FindEntity(serPDCPVar, d, in, r, rin, true);
	ptrLTENR_PDCPENTITY tarentity = LTENR_PDCP_FindEntity(tarPDCPVar, t, tin, r, rin, true);
	fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 0);

	//Buffer transfer UE PDCP
	serentity = LTENR_PDCP_FindEntity(uePDCPVar, d, in, r, rin, true);
	tarentity = LTENR_PDCP_FindEntity(uePDCPVar, t, tin, r, rin, true);
	fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 1);

	serentity = LTENR_PDCP_FindEntity(serPDCPVar, d, in, r, rin, false);
	tarentity = LTENR_PDCP_FindEntity(tarPDCPVar, t, tin, r, rin, false);
	fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 0);

	//Buffer transfer UE PDCP
	serentity = LTENR_PDCP_FindEntity(uePDCPVar, d, in, r, rin, false);
	tarentity = LTENR_PDCP_FindEntity(uePDCPVar, t, tin, r, rin, false);
	fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 1);

	//RLC Buffer transfer
	fn_NetSim_LTENR_NAS_RLC_GNB_BUFFER_TRANSFER(d, in, r, rin, t, tin);
	fn_NetSim_LTENR_NAS_RLC_UE_BUFFER_TRANSFER(d, in, r, rin, t, tin);

	//rrc reconfig remaining 
	ueRRC->SelectedCellID = hdr->targetCellID;
	ueRRC->SelectedCellIF = hdr->targetCellIF;

	//disassociation
	LTENR_PHY_ASSOCIATION(d, in, r, rin, false);
	LTENR_PDCP_ASSOCIATION(d, in, r, rin, false);
	LTENR_RLC_ASSOCIATION(d, in, r, rin, false);

	//EPC Dissociation
	ptrLTENR_EPC_HLR hlr = LTENR_EPC_FindHLR(hdr->AMFID, epcIn, hdr->UEID);
	ptrLTENR_EPC epc = LTENR_EPC_GET(hdr->AMFID, epcIn);
	LTENR_EPC_HLR_REMOVE(epc, hlr);

	hlr = LTENR_EPC_FindHLR(hdr->AMFID, epcIn, hdr->UEID);
	if (hlr != NULL) {
		LTENR_EPC_HLR_REMOVE(epc, hlr);
	}

	fn_NetSim_Stack_RemoveDeviceFromlink(r, rin, DEVICE_PHYLAYER(d, in)->nLinkId);
	fn_NetSim_Stack_AddDeviceTolink(r, rin, DEVICE_PHYLAYER(t, tin)->nLinkId);
	fn_NetSim_LTENR_RRC_RECONFIGURATION(r, rin, t, tin);
	fn_NetSim_LTENR_NAS_PATH_SWITCH_REQUEST(hdr);
}

void fn_NetSim_LTENR_NAS_HANDOVER_COMMAND_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	fn_NetSim_LTENR_NAS_HANDOVER_COMMAND_HANDLE(hdr);
}
#pragma endregion

#pragma region Handover_Request_ACK
void fn_NetSim_LTENR_NAS_HANDOVER_REQUEST_ACK(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->targetCellID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->targetCellID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = hdr->targetCellIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->targetCellID, hdr->targetCellIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_HANDOVER_REQUEST_ACK);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->targetCellID, hdr->serveringCellID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_REQUEST_ACK);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, nasHandoverRequestAck, LTENR_MSG_NAS_HANDOVER_REQUEST_ACK);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_REQUEST_ACK() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasHandoverRequestAck);
	ptrLTENR_NAS_HANDOVER_REQUEST_ACK msg = calloc(1, sizeof * msg);
	msg->GNB_UE_ID = hdr->serveringCellID;
	msg->MME_UE_ID = hdr->AMFID;
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_REQUEST_ACK;
	msg->outCome = HANDOVER_OUTCOME_SUCESSFUL;
	hdr->msg = msg;
	pstruEventDetails->pPacket->pstruMacData->dontFree = true;
	fn_NetSim_NAS_ADD_PACKET_INTO_BUUFER(msg);
}
void fn_NetSim_LTENR_NAS_HANDOVER_REQUEST_ACK_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	fn_NetSim_LTENR_NAS_HANDOVER_COMMAND(hdr);
}
#pragma endregion

#pragma region Handover_Request
void fn_NetSim_LTENR_NAS_HANDOVER_REQUEST(ptrLTENR_HANDOVER_Hdr hdr) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = hdr->serveringCellID;
	pevent.nDeviceType = DEVICE_TYPE(hdr->serveringCellID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = hdr->serveringCellIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->serveringCellID, hdr->serveringCellIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_HANDOVER_REQUEST);
	pevent.pPacket = LTENR_PACKET_CREATE(hdr->serveringCellID, hdr->targetCellID, pstruEventDetails->dEventTime, LTENR_MSG_NAS_HANDOVER_REQUEST);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, nasHandoverRequest, LTENR_MSG_NAS_HANDOVER_REQUEST);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_REQUEST() {
	ptrLTENR_HANDOVER_Hdr hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasHandoverRequest);
	ptrLTENR_NAS_HANDOVER_REQUEST msg = calloc(1, sizeof * msg);
	msg->MME_UE_ID = hdr->AMFID;
	msg->type = HANDOVER_TYPE_INTRA_LTENR;
	msg->cause = HANDOVER_CAUSE_RADIO_NETWORK;
	msg->targetID = hdr->targetCellID;
	msg->lastVisitedCellID = hdr->serveringCellID;
	msg->LastVisitedCellType = CELL_TYPE_LARGE;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_REQUEST;
	pstruEventDetails->pPacket->pstruMacData->dontFree = true;
	fn_NetSim_NAS_ADD_PACKET_INTO_BUUFER(msg);
}
void fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_REQUEST_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	fn_NetSim_LTENR_NAS_HANDOVER_REQUEST_ACK(hdr);
}
#pragma endregion

#pragma region EPC_ROUTE
void fn_NetSim_LTENR_EPC_RouteHOPacket() {
	NetSim_EVENTDETAILS pevent;
	NetSim_BUFFER* buffer;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_EPC epc = LTENR_EPC_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	memcpy(&pevent, pstruEventDetails, sizeof pevent);
	if (epc)
	{
		NETSIM_ID gnbId = get_first_dest_from_packet(pevent.pPacket);
		NETSIM_ID i;
		NETSIM_ID j, k;
		for (i = 0; i < NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->nNumOfInterface; i++)
		{
			fn_NetSim_Stack_GetConnectedDevice(pstruEventDetails->nDeviceId, i + 1, &j, &k);
			if (j == gnbId)
			{
				pevent.nInterfaceId = i + 1;
				break;
			}
		}
	}
	buffer = DEVICE_ACCESSBUFFER(pevent.nDeviceId, pevent.nInterfaceId);

	if (!fn_NetSim_GetBufferStatus(buffer))
	{
		//Add the MAC out event
		pevent.nEventType = MAC_OUT_EVENT;
		pevent.nPacketId = packet->nPacketId;
		pevent.nProtocolId = fn_NetSim_Stack_GetMacProtocol(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
		pevent.nSubEventType = 0;
		pevent.pPacket = NULL;
		pevent.szOtherDetails = NULL;
		fnpAddEvent(&pevent);
	}
	packet->nTransmitterId = pevent.nDeviceId;
	fn_NetSim_Packet_AddPacketToList(buffer, packet, 0);
	pstruEventDetails->pPacket = NULL;
}

void fn_NetSim_LTENR_EPC_RoutePacket()
{
	NetSim_BUFFER* buffer;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	//Add macout event
	buffer = DEVICE_ACCESSBUFFER(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);

	if (!fn_NetSim_GetBufferStatus(buffer))
	{
		//Add the MAC out event
		if (packet->pstruAppData)
		{
			pstruEventDetails->nApplicationId = packet->pstruAppData->nApplicationId;
			pstruEventDetails->nSegmentId = packet->pstruAppData->nSegmentId;
		}
		pstruEventDetails->nEventType = MAC_OUT_EVENT;
		pstruEventDetails->nPacketId = packet->nPacketId;
		pstruEventDetails->nProtocolId = fn_NetSim_Stack_GetMacProtocol(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket = NULL;
		pstruEventDetails->szOtherDetails = NULL;
		fnpAddEvent(pstruEventDetails);
	}
	fn_NetSim_Packet_AddPacketToList(buffer, packet, 0);
}
#pragma endregion

#pragma region NAS_INIT 
void fn_NetSim_LTENR_NAS_Init() {
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_HANDOVER_REQUEST,
		"LTENR_GENERATE_NAS_HANDOVER_REQUEST",
		fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_REQUEST);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_HANDOVER_REQUEST_ACK,
		"HANDOVER_REQUEST_ACK",
		fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_REQUEST_ACK);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_HANDOVER_COMMAND,
		"HANDOVER_REQUEST_COMMAND",
		fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_COMMAND);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_HANDOVER_COMMAND_HANDLE,
		"HANDOVER_REQUEST_COMMAND_HANDLE",
		fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_COMMAND_HANDLE);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_PATH_SWITCH,
		"PATH_SWITCH",
		fn_NetSim_LTENR_NAS_GENERATE_PATH_SWITCH_REQUEST);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_PATH_SWITCH_ACK,
		"PATH_SWITCH_ACK",
		fn_NetSim_LTENR_NAS_GENERATE_PATH_SWITCH_REQUEST_ACK);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_UECONTEXTRELEASE,
		"UE_CONTEXT_RELEASE",
		fn_NetSim_LTENR_NAS_GENERATE_UE_CONTEXT_RELEASE);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_UECONTEXTRELEASE_ACK,
		"UE_CONTEXT_RELEASE_ACK",
		fn_NetSim_LTENR_NAS_GENERATE_UE_CONTEXT_RELEASE_ACK);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_HANDOVER_COMPLETE,
		"HANDOVER_COMPLETE",
		fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_COMPLETE);
}
#pragma endregion

#pragma region NAS_IN
void fn_NetSim_NAS_IN() {
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_HANDOVER_Hdr hdr = NULL;
	switch (packet->nControlDataType) {
	case LTENR_MSG_NAS_HANDOVER_REQUEST:
		hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasHandoverRequest);
		fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_REQUEST_RECV(hdr);
		//LTENR_HDR_FREE_FROM_PACKET(packet, nasHandoverRequest);
		break;
	case LTENR_MSG_NAS_HANDOVER_REQUEST_ACK:
		hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasHandoverRequestAck);
		fn_NetSim_LTENR_NAS_HANDOVER_REQUEST_ACK_RECV(hdr);
		//LTENR_HDR_FREE_FROM_PACKET(packet, nasHandoverRequestAck);
		break;
	case LTENR_MSG_NAS_HANDOVER_COMMAND:
		hdr = LTENR_NAS_HDR_GET(packet, nasHandoverTrigger);
		fn_NetSim_LTENR_NAS_HANDOVER_COMMAND_RECV(hdr);
		//LTENR_HDR_FREE_FROM_PACKET(packet, nasHandoverTrigger);
		break;
	case LTENR_MSG_NAS_HANDOVER_PATH_SWITCH:
		hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasPathSwitch);
		fn_NetSim_LTENR_NAS_PATH_SWITCH_REQUEST_RECV(hdr);
		//LTENR_HDR_FREE_FROM_PACKET(packet, nasPathSwitch);
		break;
	case LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK:
		hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, naspathSwitchAck);
		fn_NetSim_LTENR_NAS_PATH_SWITCH_ACK_RECV(hdr);
		//LTENR_HDR_FREE_FROM_PACKET(packet, naspathSwitchAck);
		break;
	case LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE:
		hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasUEContextRelease);
		fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE_RECV(hdr);
		//LTENR_HDR_FREE_FROM_PACKET(packet, nasUEContextRelease);
		break;
	case LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK:
		hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasUEContextReleaseACK);
		fn_NetSim_LTENR_NAS_UE_CONTEXT_RELEASE_ACK_RECV(hdr);
		//LTENR_HDR_FREE_FROM_PACKET(packet, nasUEContextReleaseACK);
		break;
	case LTENR_MSG_NAS_HANDOVER_COMPLETE:
		hdr = LTENR_NAS_HDR_GET(pstruEventDetails->pPacket, nasHandoverComplete);
		fn_NetSim_LTENR_NAS_HANDOVER_COMPLETE_RECV();
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region NAS_HANDER
void fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(NetSim_PACKET* packet, ptrLTENR_HANDOVER_Hdr hdr, UINT hdrID, LTENR_MSGTYPE type) {
	LTENR_RRC_HDR_ADD_TO_PACKET(packet, hdr, hdrID, sizeof(hdr), type);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_HDR_GET(NetSim_PACKET* packet, UINT hdrID)
{
	return (ptrLTENR_HANDOVER_Hdr)LTENR_HDR_GET_FROM_PACKET(packet, hdrID);
}

void LTENR_NAS_HDR_REMOVE(NetSim_PACKET* packet, UINT hdrID)
{
	LTENR_HDR_FREE_FROM_PACKET(packet, hdrID);
}
#pragma endregion
