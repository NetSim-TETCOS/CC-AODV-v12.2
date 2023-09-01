/************************************************************************************
* Copyright (C) 2020                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Kumar Gaurav	                                                       *
*                                                                                  *
* ---------------------------------------------------------------------------------*/

#pragma region HEADER_FILE
#include "stdafx.h"
#include "LTENR_GNBRRC.h"
#include "LTENR_NAS.h"
#pragma endregion

#pragma region INIT_AND_REGISTER_CALLBACK_FUN
static void LTENR_GNBRRC_register_callback_function()
{
	fn_NetSim_LTENR_RRC_TimerEvent_Init();
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVNET_GENERATE_MIB,
		"LTENR_GENERATE_RRC_MIB",
		fn_NetSim_LTENR_GNBRRC_GenerateMIB);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_SIB1,
		"LTENR_GENERATE_RRC_SIB1",
		fn_NetSim_LTENR_GNBRRC_GenerateSIB1);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_SI,
		"LTENR_GENERATE_RRC_SI",
		fn_NETSIM_LTENR_SUBEVENT_GENERATE_SI);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_RRC_SETUP_REQUEST,
		"LTENR_GENERATE_RRC_SETUP_REQUEST",
		fn_NetSim_LTENR_RRC_GENERATE_RRC_CONN_REQUEST);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_RRC_SETUP,
		"LTENR_GENERATE_RRC_SETUP",
		fn_NetSIM_LTENR_RRC_GENERATE_RRCSETUP);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_RRC_SETUP_COMPLETE,
		"LTENR_GENERATE_RRC_SETUP_COMPLETE",
		fn_NetSim_LTENR_RRC_GENERATE_RRC_SETUP_COMPLETE);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_RRC_UE_MEASUREMENT_REPORT,
		"LTENR_GENERATE_RRC_UE_MEASUREMENT_REPORT",
		fn_NetSIM_LTENR_RRC_GENERATE_UE_MEASUREMENT_REPORT);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_RRC_UE_MEASUREMENT_REPORT_REQUEST,
		"LTENR_GENERATE_RRC_UE_MEASUREMENT_REPORT_REQEST",
		fn_NetSim_LTENR_RRC_UE_Measurement_Report_Request_Handle);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_RRC_RECONFIGURATION,
		"LTENR_GENERATE_RRC_UE_MEASUREMENT_REPORT",
		fn_NetSIM_LTENR_RRC_GENERATE_RECONFIGURATION);
}

void fn_NetSim_LTENR_RRC_Init()
{
	LTENR_GNBRRC_register_callback_function();
	LTENR_RRCMSG_INIT();
}
#pragma endregion

#pragma region MACOUT_AND_MACIN
void fn_NetSim_LTENR_RRC_MacOut()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	packet->nReceiverId = LTENR_FIND_ASSOCIATEDGNB(d, in);
	LTENR_CallPDCPOut();
}

void fn_NetSim_LTENR_RRC_MacIn()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_RRC_Hdr hdr = NULL;

	switch (packet->nControlDataType) {
	case LTENR_MSG_RRC_MIB:
		hdr = LTENR_RRC_HDR_GET(packet, mibHdrID);
		fn_NetSim_LTENR_UERRC_MIB_RECV(d, in, hdr->SenderID, hdr->SenderIF, hdr->msg);
		LTENR_HDR_FREE_FROM_PACKET(packet, mibHdrID);
		break;
	case LTENR_MSG_RRC_SIB1:
		hdr = LTENR_RRC_HDR_GET(packet, sib1HdrID);
		fn_NetSim_LTENR_UERRC_SIB1_RECV(d, in, hdr->SenderID, hdr->SenderIF, hdr->msg);
		LTENR_HDR_FREE_FROM_PACKET(packet, sib1HdrID);
		break;
	case LTENR_MSG_RRC_SI:
		hdr = LTENR_RRC_HDR_GET(packet, siHdrID);
		fn_NetSim_LTENR_RRC_SI_RECV(d, in, hdr->SenderID, hdr->SenderIF, hdr->msg);
		LTENR_HDR_FREE_FROM_PACKET(packet, siHdrID);
		break;
	case LTENR_MSG_RRC_SETUP_REQUEST:
		hdr = LTENR_RRC_HDR_GET(packet, rrcSetupRequestID);
		fn_NetSim_LTENR_RRC_SETUP_REQUEST_RECV(d, in, hdr->SenderID, hdr->SenderIF, hdr->msg);
		LTENR_HDR_FREE_FROM_PACKET(packet, rrcSetupRequestID);
		break;
	case LTENR_MSG_RRC_SETUP:
		hdr = LTENR_RRC_HDR_GET(packet, rrcSetupID);
		fn_NetSim_LTENR_RRC_SETUP_RECV(d, in, hdr->SenderID, hdr->SenderIF, hdr->msg);
		LTENR_HDR_FREE_FROM_PACKET(packet, rrcSetupID);
		break;
	case LTENR_MSG_RRC_SETUP_COMPLETE:
		hdr = LTENR_RRC_HDR_GET(packet, rrcSetupCompleteID);
		fn_NetSim_LTENR_RRC_RRCSETUPCOMPLETE_RECV(d, in, hdr->SenderID, hdr->SenderIF);
		LTENR_HDR_FREE_FROM_PACKET(packet, rrcSetupCompleteID);
		break;
	case LTENR_MSG_RRC_UE_MEASUREMENT_REPORT:
		hdr = LTENR_RRC_HDR_GET(packet, ueMEASID);
		fn_NetSim_LTENR_RRC_UE_MEASUREMENT_RECV(d, in, hdr->SenderID, hdr->SenderIF, hdr->msg);
		LTENR_HDR_FREE_FROM_PACKET(packet, ueMEASID);
		break;
	case LTENR_MSG_RRC_RECONFIGURATION:
		hdr = LTENR_RRC_HDR_GET(packet, rrcReconfigurationID);
		fn_NetSim_LTENR_RRC_RECONFIGURATION_RECV(d, in, hdr->SenderID, hdr->SenderIF, hdr->msg);
		LTENR_HDR_FREE_FROM_PACKET(packet, rrcReconfigurationID);
		break;
	default:
		break;

	}
}
#pragma endregion

#pragma region ISRRCCONNECTED
bool fn_NetSim_LTENR_RRC_isConnected(NETSIM_ID d, NETSIM_ID in,
	NETSIM_ID r, NETSIM_ID rin)
{
	if (isUE(d, in)) {
		ptrLTENR_UERRC rrc = LTENR_UERRC_GET(d, in);
		if (rrc->ueRRCState == UERRC_CONNECTED) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (isUE(r, rin)) {
			ptrLTENR_UERRC rrc = LTENR_UERRC_GET(r, rin);
			if (rrc->ueRRCState == UERRC_CONNECTED) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}
#pragma endregion

#pragma region LTENR_PHY_MOBILITY_HANDLE
void fn_NetSim_PHY_MOBILITY_HANDLE(NETSIM_ID d) {
	NETSIM_ID in = fn_NetSim_Get_LTENR_INTERFACE_ID_FROM_DEVICE_ID(d, LTENR_DEVICETYPE_UE);
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(d, in);
	if (rrc->ueRRCState == UERRC_CONNECTED) {
		fn_NetSim_UPDATE_SPECTRAL_EFFICIENCY_INFO(rrc->SelectedCellID, rrc->SelectedCellIF, d, in);
	}
}
#pragma endregion
