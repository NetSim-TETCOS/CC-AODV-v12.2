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
#include "LTENR_MAC.h"

#pragma region MAC_INIT
void fn_NetSim_LTENR_MAC_Init()
{
}

void fn_NetSim_LTENR_UEMAC_Init(NETSIM_ID ueId, NETSIM_ID ueIf)
{
}

static LTENR_GnbSchedulingType fn_NetSim_LTENR_SchedulingType(char* var) {
	if (!_stricmp(var, "ROUND_ROBIN"))
		return LTENR_MAC_SCHEDULING_ROUNDROBIN;
	else if (!_stricmp(var, "PROPORTIONAL_FAIR"))
		return LTENR_MAC_SCHEDULING_PROPORTIONALFAIR;
	else if (!_stricmp(var, "MAX_THROUGHPUT"))
		return LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT;
	else if (!_stricmp(var, "FAIR_SCHEDULING"))
		return LTENR_MAC_SCHEDULING_FAIR_SCHEDULING;
	else {
		fnNetSimError("Unknown Scheduling Type");
		return LTENR_MAC_SCHEDULING_ROUNDROBIN;
	}
}

void fn_NetSim_LTENR_GNBMAC_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBMAC mac = calloc(1, sizeof * mac);
	mac->gnbId = gnbId;
	mac->gnbIf = gnbIf;
	LTENR_GNBMAC_SET(gnbId, gnbIf, mac);

	LTENR_GNB_SETEPC(gnbId, gnbIf, &mac->epcId, &mac->epcIf);
}
#pragma endregion

#pragma region MAC_TEMP_FUNCTION

void macOut()
{
	pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;
	fnpAddEvent(pstruEventDetails);
}

void macIn()
{
	LTENR_CallRLCIn();
}
#pragma endregion

#pragma region MAC_SCHEDULARINTERFACE
static ptrLTENR_UESCHEDULARINFO LTENR_MACSchedular_FindInfoForUE(ptrLTENR_SCHEDULARINFO si,
																 NETSIM_ID d, NETSIM_ID in,
																 bool isUplink)
{
	ptrLTENR_UESCHEDULARINFO info = isUplink ? si->uplinkInfo : si->downlinkInfo;
	while (info)
	{
		if (info->ueId == d && info->ueIf == in)
			return info;
		info = info->next;
	}
	return NULL;
}

static ptrLTENR_UESCHEDULARINFO LTENR_MACSchedular_InitInfoForUE(ptrLTENR_SCHEDULARINFO si,
																 NETSIM_ID d, NETSIM_ID in,
																 bool isUplink)
{
	ptrLTENR_UESCHEDULARINFO info = calloc(1, sizeof * info);
	info->ueId = d;
	info->ueIf = in;
	ptrLTENR_UESCHEDULARINFO l = isUplink ? si->uplinkInfo : si->downlinkInfo;
	if (l)
	{
		while (l->next)
			l = l->next;
		l->next = info;
	}
	else
	{
		if (isUplink) si->uplinkInfo = info;
		else si->downlinkInfo = info;
	}
	return info;
}

static void LTENR_MACSchedular_UpdateInfoForUE(ptrLTENR_GNBMAC mac, ptrLTENR_GNBPHY phy,
											   ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo,
											   ptrLTENR_UESCHEDULARINFO info, bool isUplink, int CA_ID)
{
	ptrLTENR_AMCINFO amc = isUplink ? assocInfo->uplinkAMCInfo[CA_ID] : assocInfo->downlinkAMCInfo[CA_ID];
	info->cqiIndex = amc->cqiTable.CQIIndex;
	info->bitsPerPRB = LTENR_calculateTBSSize(phy, 1, isUplink, amc->mcsTable, CA_ID);
	if (isUplink)
		info->bufferSize = fn_NetSim_LTENR_RLC_BufferStatusNotificaton(assocInfo->ueId, assocInfo->ueIf,
																	   mac->gnbId, mac->gnbIf,
																	   LTENR_LOGICALCHANNEL_DTCH);
	else
		info->bufferSize = fn_NetSim_LTENR_RLC_BufferStatusNotificaton(mac->gnbId, mac->gnbIf,
																	   assocInfo->ueId, assocInfo->ueIf,
																	   LTENR_LOGICALCHANNEL_DTCH);
}

static void LTENR_MAC_AddSchedularStausForUE(ptrLTENR_GNBMAC mac, ptrLTENR_GNBPHY phy,
											 ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, ptrLTENR_SCHEDULARINFO si, int CA_ID)
{
	ptrLTENR_UESCHEDULARINFO uinfo = LTENR_MACSchedular_FindInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, true);
	ptrLTENR_UESCHEDULARINFO dinfo = LTENR_MACSchedular_FindInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, false);
	if (!uinfo)
		uinfo = LTENR_MACSchedular_InitInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, true);
	if (!dinfo)
		dinfo = LTENR_MACSchedular_InitInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, false);

	LTENR_MACSchedular_UpdateInfoForUE(mac, phy, assocInfo, uinfo, true, CA_ID);
	LTENR_MACSchedular_UpdateInfoForUE(mac, phy, assocInfo, dinfo, false, CA_ID);
} 

static void LTENR_MAC_InitSchedularStatus(ptrLTENR_GNBMAC mac)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(mac->gnbId, mac->gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	ptrLTENR_SCHEDULARINFO info = NULL;
	if (mac->schedularInfo[CA_ID])
	{
		info = mac->schedularInfo[CA_ID];
		info->slotType = phy->currentFrameInfo->slotType;

		ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(mac->gnbId, mac->gnbIf);
		info->schedulingType = fn_NetSim_LTENR_SchedulingType(pd->schedulingType);
	}
	else
	{
		info = calloc(1, sizeof * info);
		mac->schedularInfo[CA_ID] = info;
		info->gnbId = mac->gnbId;
		info->gnbIf = mac->gnbIf;
		info->PRBCount = ca->PRBCount;
		LTENR_PHY_GET_OH(sc, &info->OH_DL, &info->OH_UL);
		info->slotType = phy->currentFrameInfo->slotType;

		ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(mac->gnbId, mac->gnbIf);
		info->schedulingType = fn_NetSim_LTENR_SchedulingType(pd->schedulingType);
	}

	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
	while (assocInfo)
	{
		LTENR_MAC_AddSchedularStausForUE(mac, phy, assocInfo, info, CA_ID);
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
	}

}
#pragma endregion

#pragma region MAC_RLC_INTERFACE
static void LTENR_MAC_NotifyRLCForTransmission_forControl(NETSIM_ID d, NETSIM_ID in,
														  NETSIM_ID r, NETSIM_ID rin)
{
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_BCCH);
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_CCCH);
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_DCCH);
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_PCCH);
}

static void LTENR_MAC_NotifyRLCForTransmission_forDTCH(NETSIM_ID d, NETSIM_ID in,
													   ptrLTENR_UESCHEDULARINFO si, bool isUplink)
{
	while (si)
	{
		if (isUplink)
			fn_NetSim_LTENR_RLC_TransmissionStatusNotification(si->ueId, si->ueIf,
															   d, in, (UINT)(si->allocatedPRBCount*si->bitsPerPRB/8.0),
															   LTENR_LOGICALCHANNEL_DTCH);
		else
			fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
															   si->ueId, si->ueIf,
				(UINT)(si->allocatedPRBCount * si->bitsPerPRB / 8.0),
															   LTENR_LOGICALCHANNEL_DTCH);
		si = si->next;
	}
}

void LTENR_MAC_NotifyRLCForTransmission(ptrLTENR_GNBMAC mac)
{
	NETSIM_ID gnbId = mac->gnbId;
	NETSIM_ID gnbIf = mac->gnbIf;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	ptrLTENR_SCHEDULARINFO si = mac->schedularInfo[CA_ID];
	
	LTENR_SLOTTYPE slotType = si->slotType;

	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
	while (assocInfo)
	{
		if (slotType == SLOT_UPLINK || slotType == SLOT_MIXED)
			LTENR_MAC_NotifyRLCForTransmission_forControl(assocInfo->ueId, assocInfo->ueIf,
														  gnbId, gnbIf);

		if (slotType == SLOT_DOWNLINK || slotType == SLOT_MIXED)
			LTENR_MAC_NotifyRLCForTransmission_forControl(gnbId, gnbIf,
														  assocInfo->ueId, assocInfo->ueIf);


		LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
	}
	if (slotType == SLOT_DOWNLINK || slotType == SLOT_MIXED)
		LTENR_MAC_NotifyRLCForTransmission_forControl(gnbId, gnbIf,
			0, 0);//for rrc MIB and SIB1 broadcast msg

	if (slotType == SLOT_UPLINK || slotType == SLOT_MIXED)
		LTENR_MAC_NotifyRLCForTransmission_forDTCH(gnbId, gnbIf, si->uplinkInfo, true);

	if (slotType == SLOT_DOWNLINK || slotType == SLOT_MIXED)
		LTENR_MAC_NotifyRLCForTransmission_forDTCH(gnbId, gnbIf, si->downlinkInfo, false);
	
}
#pragma endregion

#pragma region MAC_PHY_INTERFACE
void LTENR_NotifyMACForStartingSlot()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	LTENR_MAC_InitSchedularStatus(mac);
	//schedular call
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	LTENR_PRB_Schedular(mac->schedularInfo[CA_ID]);

	//print_ltenr_log("Allocated PRB = %d and CA_ID= %d\n", mac->schedularInfo[CA_ID]->downlinkInfo->allocatedPRBCount, CA_ID);

	//notify RLC for transmission status
	LTENR_MAC_NotifyRLCForTransmission(mac);
}
#pragma endregion
