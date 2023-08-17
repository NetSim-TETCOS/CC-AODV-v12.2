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
#include "LTENR_MAC.h"
#include "LTENR_PHY.h"

#pragma region FUNCTION_PROTOTYPE
//PRB
static void LTENR_form_prb_list(NETSIM_ID d, NETSIM_ID in, int CA_ID);

//Frame
static void LTENR_addStartFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID);
void LTENR_handleStartFrameEvent();

//SubFrame
static void LTENR_addStartSubFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID);
void LTENR_handleStartSubFrameEvent();

//Slot
static void LTENR_addStartSlotEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID);
void LTENR_handleStartSlotEvent();

//Association
void LTENR_PHY_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
						   NETSIM_ID ueId, NETSIM_ID ueIf,
						   bool isAssociated);
//AMC
static void LTENR_PHY_initAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo);
static void LTENR_PHY_setAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO info, int CA_ID);
#pragma endregion

#pragma region PHY_INIT
void fn_NetSim_LTENR_PHY_Init()
{
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTFRAME, "LTENR_STARTFRAME", LTENR_handleStartFrameEvent);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTSUBFRAME, "LTENR_STARTSUBFRAME", LTENR_handleStartSubFrameEvent);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTSLOT, "LTENR_STARTSLOT", LTENR_handleStartSlotEvent);

	fn_NetSim_LTENR_RegisterCallBackForAssociation(LTENR_PHY_ASSOCIATION);
}

void fn_NetSim_LTENR_UEPHY_Init(NETSIM_ID ueId, NETSIM_ID ueIf)
{
}

void fn_NetSim_LTENR_GNBPHY_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	UINT i = 0;
	for (i = 0; i < phy->ca_count; i++) {
		phy->spectrumConfig->Current_CA_ID = i;

		LTENR_form_prb_list(gnbId, gnbIf, i);
		LTENR_addStartFrameEvent(gnbId, gnbIf, 0, i);
#pragma warning (disable : 4312)
		pstruEventDetails->szOtherDetails = (void*)i;
#pragma warning (default : 4312)
		ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(gnbId, gnbIf, 0, 0);
		while (info)
		{
			//LTENR_PHY_ASSOCIATION(gnbId, gnbIf,
			//	info->d, info->in,
			//	true);
			info = info->next;
		}
	}
}

ptrLTENR_GNBPHY LTENR_GNBPHY_NEW(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBPHY phy = calloc(1, sizeof * phy);
	phy->spectrumConfig = calloc(1, sizeof * phy->spectrumConfig);
	phy->propagationConfig = calloc(1, sizeof * phy->propagationConfig);
	LTENR_GNBPHY_SET(gnbId, gnbIf, phy);
	phy->gnbId = gnbId;
	phy->gnbIf = gnbIf;
	return phy;
}

ptrLTENR_UEPHY LTENR_UEPHY_NEW(NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_UEPHY phy = calloc(1, sizeof * phy);
	phy->propagationConfig = calloc(1, sizeof * phy->propagationConfig);
	LTENR_UEPHY_SET(ueId, ueIf, phy);
	phy->ueId = ueId;
	phy->ueIf = ueIf;
	return phy;
}
#pragma endregion

#pragma region PRB
static void LTENR_form_prb_list(NETSIM_ID d, NETSIM_ID in, int CA_ID)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(d, in);
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	ptrLTENR_CA ca = sc->CA[CA_ID];
	UINT i;
	print_ltenr_log("Forming PRB list for gNB %d:%d -- \n", d, in);
	double prbBandwidth_kHz = ca->prbBandwidth_kHz;
	double guard_kHz = ca->guardBand_kHz;
	print_ltenr_log("CA_ID= %d\n", CA_ID);
	print_ltenr_log("\tF_Low_MHz = %d\n", ca->Flow_MHZ);
	print_ltenr_log("\tF_High_MHz = %d\n", ca->Fhigh_MHz);
	print_ltenr_log("\tChannel_Bandwidth_MHz = %lf\n", ca->channelBandwidth_mHz);
	print_ltenr_log("\tPRB_Bandwidth_kHz = %lf\n", prbBandwidth_kHz);
	print_ltenr_log("\tGuard_Bandwidth_kHz = %lf\n", guard_kHz);
	print_ltenr_log("\tPRB_ID\tF_Low\tF_High\tF_center\n");

	phy->prbList = calloc(ca->PRBCount, sizeof * phy->prbList);
	for (i = 0; i < ca->PRBCount; i++)
	{
		phy->prbList[i].prbId = i + 1;
		phy->prbList[i].lFrequency_MHz = ca->Flow_MHZ + guard_kHz * 0.001 + prbBandwidth_kHz * i * 0.001;
		phy->prbList[i].uFrequency_MHz = phy->prbList[i].lFrequency_MHz + prbBandwidth_kHz * 0.001;
		phy->prbList[i].centralFrequency_MHz = (phy->prbList[i].lFrequency_MHz + phy->prbList[i].uFrequency_MHz) / 2.0;
		phy->prbList[i].prbBandwidth_MHz = prbBandwidth_kHz * 0.001;
		print_ltenr_log("\t%3d\t%lf\t%lf\t%lf\n",
						i + 1,
						phy->prbList[i].lFrequency_MHz,
						phy->prbList[i].uFrequency_MHz,
						phy->prbList[i].centralFrequency_MHz);
	}
	print_ltenr_log("\n\n");
}
#pragma endregion

#pragma region FRAME
static void LTENR_addStartFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PHY_STARTFRAME);
#pragma warning (disable : 4312)
	pevent.szOtherDetails = (void*)CA_ID;
#pragma warning (default : 4312)
	fnpAddEvent(&pevent);
}

static void LTENR_resetFrame(ptrLTENR_GNBPHY phy, int CA_ID)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo[CA_ID];
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

	info->frameId++;
	info->frameStartTime = pstruEventDetails->dEventTime;
	info->frameEndTime = pstruEventDetails->dEventTime + sc->frameDuration;

	//reset slot
	info->slotEndTime = 0;
	info->slotId = 0;
	info->slotStartTime = 0;

	//reset subframe
	info->subFrameEndTime = 0;
	info->subFrameId = 0;
	info->subFrameStartTime = 0;
}

void LTENR_handleStartFrameEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	
	LTENR_resetFrame(phy, CA_ID);
	print_ltenr_log("Starting new frame for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("CA_ID for Frame = %d\n", CA_ID);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo[CA_ID]->frameId);
	print_ltenr_log("\tFrame start time (us) = %lf\n", phy->frameInfo[CA_ID]->frameStartTime);
	print_ltenr_log("\tFrame end time (us) = %lf\n", phy->frameInfo[CA_ID]->frameEndTime);

	LTENR_addStartFrameEvent(gnbId, gnbIf,
							 phy->frameInfo[CA_ID]->frameEndTime, CA_ID);

	LTENR_addStartSubFrameEvent(gnbId, gnbIf,
								phy->frameInfo[CA_ID]->frameStartTime, CA_ID);
}
#pragma endregion

#pragma region SUBFRAME
static void LTENR_addStartSubFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PHY_STARTSUBFRAME);
#pragma warning (disable : 4312)
	pevent.szOtherDetails = (void*)CA_ID;
#pragma warning (default : 4312)
	fnpAddEvent(&pevent);
}

static void LTENR_resetSubFrame(ptrLTENR_GNBPHY phy, int CA_ID)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo[CA_ID];
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

	//reset slot
	info->slotEndTime = 0;
	info->slotId = 0;
	info->slotStartTime = 0;

	//reset subframe
	info->subFrameStartTime = pstruEventDetails->dEventTime;
	info->subFrameEndTime = pstruEventDetails->dEventTime + sc->subFrameDuration;
	info->subFrameId++;
	
}

void LTENR_handleStartSubFrameEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)
	
	LTENR_resetSubFrame(phy,CA_ID);
	print_ltenr_log("Starting new sub frame for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("CA_ID for SubFrame = %d\n", CA_ID);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo[CA_ID]->frameId);
	print_ltenr_log("\tSubFrame Id = %d\n", phy->frameInfo[CA_ID]->subFrameId);
	print_ltenr_log("\tSubFrame start time (us) = %lf\n", phy->frameInfo[CA_ID]->subFrameStartTime);
	print_ltenr_log("\tSubFrame end time (us) = %lf\n", phy->frameInfo[CA_ID]->subFrameEndTime);

	if (phy->frameInfo[CA_ID]->subFrameId != phy->spectrumConfig->subFramePerFrame)
		LTENR_addStartSubFrameEvent(gnbId, gnbIf,
									phy->frameInfo[CA_ID]->subFrameEndTime, CA_ID);

	LTENR_addStartSlotEvent(gnbId, gnbIf,
								phy->frameInfo[CA_ID]->subFrameStartTime, CA_ID);
}
#pragma endregion

#pragma region SLOT
static void LTENR_addStartSlotEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PHY_STARTSLOT);
#pragma warning (disable : 4312)
	pevent.szOtherDetails = (PVOID)CA_ID;
#pragma warning (default : 4312)	
	fnpAddEvent(&pevent);
}

static void LTENR_resetSlot(ptrLTENR_GNBPHY phy, int CA_ID)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo[CA_ID];
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];

	//reset slot
	info->slotId++;
	info->slotStartTime = pstruEventDetails->dEventTime;
	info->slotEndTime = pstruEventDetails->dEventTime + ca->slotDuration_us;

	if (ca->configSlotType == SLOT_MIXED)
	{
		if (ca->totalSlotCount && (ca->dlSlotCount*1.0) / ca->totalSlotCount < ca->dlSlotRatio)
		{
			ca->dlSlotCount++;
			info->slotType = SLOT_DOWNLINK;
		}
		else
			info->slotType = SLOT_UPLINK;

		ca->totalSlotCount++;


		//if (info->prevSlotType == SLOT_UPLINK)
		//	info->slotType = SLOT_DOWNLINK;
		//else if (info->prevSlotType == SLOT_DOWNLINK)
		//	info->slotType = SLOT_UPLINK;
		//else
		//	info->slotType = SLOT_DOWNLINK;
	}
	else
		info->slotType = ca->configSlotType;
}

void LTENR_handleStartSlotEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)

	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];

	LTENR_resetSlot(phy,CA_ID);
	print_ltenr_log("Starting new slot for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("CA_ID for Slot = %d\n", CA_ID);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo[CA_ID]->frameId);
	print_ltenr_log("\tSubFrame Id = %d\n", phy->frameInfo[CA_ID]->subFrameId);
	print_ltenr_log("\tSlot Id = %d\n", phy->frameInfo[CA_ID]->slotId);
	print_ltenr_log("\tSlot start time (us) = %lf\n", phy->frameInfo[CA_ID]->slotStartTime);
	print_ltenr_log("\tslot end time (us) = %lf\n", phy->frameInfo[CA_ID]->slotEndTime);
	print_ltenr_log("\tSlot type = %s\n", strLTENR_SLOTTYPE[phy->frameInfo[CA_ID]->slotType]);

	phy->currentFrameInfo = phy->frameInfo[CA_ID];
	phy->currentFrameInfo->Current_CA_ID = CA_ID;
	if (phy->frameInfo[CA_ID]->slotId != ca->slotPerSubframe)
		LTENR_addStartSlotEvent(gnbId, gnbIf,
								phy->frameInfo[CA_ID]->slotEndTime, CA_ID);

	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	while (info) {
		NETSIM_ID i = 0;
		for (i = 0; i < phy->ca_count; i++) {
			LTENR_PHY_setAMCInfo(phy, info, i);
		}
		info = LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	}

	LTENR_NotifyMACForStartingSlot();

	phy->frameInfo[CA_ID]->prevSlotType = phy->frameInfo[CA_ID]->slotType;
}
#pragma endregion

#pragma region PHY_PROPAGATION_INTERFACE
static void LTENR_PHY_initPropagationInfo(ptrLTENR_GNBPHY phy,
										  ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, int CA_ID)
{
	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(assocInfo->ueId, assocInfo->ueIf);
	ptrLTENR_PROPAGATIONINFO info = calloc(1, sizeof * info);
	assocInfo->propagationInfo[CA_ID] = info;
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];

	info->gnbId = phy->gnbId;
	info->gnbIf = phy->gnbIf;
	memcpy(&info->gnbPos, DEVICE_POSITION(phy->gnbId), sizeof info->gnbPos);
	memcpy(&info->uePos, DEVICE_POSITION(assocInfo->ueId), sizeof info->uePos);
	info->ueId = assocInfo->ueId;
	info->ueIf = assocInfo->ueIf;
	info->bandwidth_MHz = ca->channelBandwidth_mHz;
	info->centralFrequency_MHz = (ca->Fhigh_MHz + ca->Flow_MHZ) / 2;
	info->downlink.txPower_dbm = phy->propagationConfig->txPower_dbm;
	info->uplink.txPower_dbm = uePhy->propagationConfig->txPower_dbm;

	info->propagationConfig = phy->propagationConfig;
	info->propagationConfig->UE_height = uePhy->propagationConfig->UE_height;
	info->propagationConfig->beamFormingGain = uePhy->propagationConfig->beamFormingGain;
}

static void LTENR_PHY_updatePropagationInfo(ptrLTENR_GNBPHY phy,
											ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, int CA_ID)
{
	ptrLTENR_PROPAGATIONINFO info = assocInfo->propagationInfo[CA_ID];
	memcpy(&info->gnbPos, DEVICE_POSITION(phy->gnbId), sizeof info->gnbPos);
	memcpy(&info->uePos, DEVICE_POSITION(assocInfo->ueId), sizeof info->uePos);
}

#define BOLTZMANN 1.38064852e-23 //m2kgs-2K-1
#define TEMPERATURE 300 //kelvin
static double LTENR_PHY_calculateThermalNoise(double bandwidth)
{
	double noise = BOLTZMANN * TEMPERATURE * bandwidth * 1000000; //in W
	noise *= 1000; // in mW
	double noise_dbm = MW_TO_DBM(noise);
	return noise_dbm;
}

static void LTENR_PHY_calculateSNR(ptrLTENR_PROPAGATIONINFO info)
{
	//uplink
	double p = DBM_TO_MW(info->uplink.rxPower_dbm);
	double n = DBM_TO_MW(info->uplink.thermalNoise);
	info->uplink.EB_by_N0 = p / n;
	info->uplink.SNR_db = MW_TO_DBM(info->uplink.EB_by_N0);

	//downlink
	p = DBM_TO_MW(info->downlink.rxPower_dbm);
	n = DBM_TO_MW(info->downlink.thermalNoise);
	info->downlink.EB_by_N0 = p / n;
	info->downlink.SNR_db = MW_TO_DBM(info->downlink.EB_by_N0);
}

static void LTENR_PHY_calculateSpectralEfficiency(ptrLTENR_GNBPHY phy,
												  ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, int CA_ID)
{
	if (!assocInfo->propagationInfo[CA_ID])
		LTENR_PHY_initPropagationInfo(phy, assocInfo, CA_ID);
	else
		LTENR_PHY_updatePropagationInfo(phy, assocInfo, CA_ID);

	ptrLTENR_PROPAGATIONINFO info = assocInfo->propagationInfo[CA_ID];

	//Call propagation model
	print_ltenr_log("\n\tCarrier Id =%d\n", CA_ID);
	LTENR_Propagation_TotalLoss(info);

	info->uplink.rxPower_dbm = info->uplink.txPower_dbm - info->dTotalLoss;
	info->downlink.rxPower_dbm = info->downlink.txPower_dbm - info->dTotalLoss;
	info->uplink.thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
	info->downlink.thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
	LTENR_PHY_calculateSNR(info);
	info->downlink.spectralEfficiency = log2(1 + info->downlink.EB_by_N0);
	info->uplink.spectralEfficiency = log2(1 + info->uplink.EB_by_N0);

	print_ltenr_log("\tDL Thermal Noise = %lfdB\n", info->downlink.thermalNoise);
	print_ltenr_log("\tDL Signal to Noise Ratio (SNR) = %lfdB\n", info->downlink.SNR_db);
	print_ltenr_log("\tDL Spectral Efficiency = %lfdB\n", info->downlink.spectralEfficiency);
	print_ltenr_log("\tUL Thermal Noise = %lfdB\n", info->uplink.thermalNoise);
	print_ltenr_log("\tUL Signal to Noise Ratio (SNR) = %lfdB\n", info->uplink.SNR_db);
	print_ltenr_log("\tUL Spectral Efficiency = %lfdB\n", info->uplink.spectralEfficiency);
	print_ltenr_log("\n");
}

double LTENR_PHY_Send_SINR_to_RRC(ptrLTENR_GNBPHY phy) {
	UINT i;
	double sinr = 0;
	for (i = 0; i < phy->ca_count; i++) {
		 sinr += phy->associatedUEPhyInfo->propagationInfo[i]->downlink.SNR_db;
	}
	return sinr/phy->ca_count;
}

double LTENR_PHY_Send_RSRP_to_RRC(ptrLTENR_GNBPHY phy) {
	UINT i;
	double rsrp = 0;
	for (i = 0; i < phy->ca_count; i++) {
		rsrp += phy->associatedUEPhyInfo->propagationInfo[i]->downlink.rxPower_dbm;
	}
	return rsrp/phy->ca_count;
}

static void LTENR_PHY_setAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO info, int CA_ID)
{
	//Downlink
	info->downlinkAMCInfo[CA_ID]->SpectralEfficiency = info->propagationInfo[CA_ID]->downlink.spectralEfficiency;
	info->downlinkAMCInfo[CA_ID]->cqiTable = LTENR_GetCQITableFromSpectralEfficiency(phy->CSIReportConfig.cqiTable,
		info->downlinkAMCInfo[CA_ID]->SpectralEfficiency);
	info->downlinkAMCInfo[CA_ID]->mcsTable = LTENR_GetMCSIndexTableFromSpectralEfficiency(phy->PDSCHConfig.mcsTable,
		info->downlinkAMCInfo[CA_ID]->cqiTable.efficiency);

	print_ltenr_log("\tAMC info between gNB %d:%d and UE %d:%d for downlink-\n",
		phy->gnbId, phy->gnbIf,
		info->ueId, info->ueIf);
	print_ltenr_log("\t\tSpectral Efficiency = %lf\n", info->downlinkAMCInfo[CA_ID]->SpectralEfficiency);
	print_ltenr_log("\t\tCQI Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\n",
		info->downlinkAMCInfo[CA_ID]->cqiTable.CQIIndex,
		strPHY_MODULATION[info->downlinkAMCInfo[CA_ID]->cqiTable.modulation],
		info->downlinkAMCInfo[CA_ID]->cqiTable.codeRate,
		info->downlinkAMCInfo[CA_ID]->cqiTable.efficiency);
	print_ltenr_log("\t\tMCS Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\t%lf\n",
		info->downlinkAMCInfo[CA_ID]->mcsTable.mcsIndex,
		strPHY_MODULATION[info->downlinkAMCInfo[CA_ID]->mcsTable.modulation],
		info->downlinkAMCInfo[CA_ID]->mcsTable.modulationOrder,
		info->downlinkAMCInfo[CA_ID]->mcsTable.codeRate,
		info->downlinkAMCInfo[CA_ID]->mcsTable.spectralEfficiency);

	//Uplink
	info->uplinkAMCInfo[CA_ID]->SpectralEfficiency = info->propagationInfo[CA_ID]->uplink.spectralEfficiency;
	info->uplinkAMCInfo[CA_ID]->cqiTable = LTENR_GetCQITableFromSpectralEfficiency(phy->CSIReportConfig.cqiTable,
		info->uplinkAMCInfo[CA_ID]->SpectralEfficiency);
	info->uplinkAMCInfo[CA_ID]->mcsTable = LTENR_GetMCSIndexTableFromSpectralEfficiency(phy->PUSCHConfig.mcsTable,
		info->uplinkAMCInfo[CA_ID]->cqiTable.efficiency);
	print_ltenr_log("\tAMC info between gNB %d:%d and UE %d:%d for uplink-\n",
		phy->gnbId, phy->gnbIf,
		info->ueId, info->ueIf);
	print_ltenr_log("\t\tSpectral Efficiency = %lf\n", info->uplinkAMCInfo[CA_ID]->SpectralEfficiency);
	print_ltenr_log("\t\tCQI Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\n",
		info->uplinkAMCInfo[CA_ID]->cqiTable.CQIIndex,
		strPHY_MODULATION[info->uplinkAMCInfo[CA_ID]->cqiTable.modulation],
		info->uplinkAMCInfo[CA_ID]->cqiTable.codeRate,
		info->uplinkAMCInfo[CA_ID]->cqiTable.efficiency);
	print_ltenr_log("\t\tMCS Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\t%lf\n",
		info->uplinkAMCInfo[CA_ID]->mcsTable.mcsIndex,
		strPHY_MODULATION[info->uplinkAMCInfo[CA_ID]->mcsTable.modulation],
		info->uplinkAMCInfo[CA_ID]->mcsTable.modulationOrder,
		info->uplinkAMCInfo[CA_ID]->mcsTable.codeRate,
		info->uplinkAMCInfo[CA_ID]->mcsTable.spectralEfficiency);
}
#pragma endregion

#pragma region PHY_AMCINFO
static void LTENR_PHY_initAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo)
{
	NETSIM_ID i = 0;
	for (i = 0; i < phy->ca_count; i++) {
		if (!assocInfo->downlinkAMCInfo[i])
			assocInfo->downlinkAMCInfo[i] = calloc(1, sizeof * assocInfo->downlinkAMCInfo[i]);
		if (!assocInfo->uplinkAMCInfo[i])
			assocInfo->uplinkAMCInfo[i] = calloc(1, sizeof * assocInfo->uplinkAMCInfo[i]);

		LTENR_PHY_calculateSpectralEfficiency(phy, assocInfo, i);
		LTENR_PHY_setAMCInfo(phy, assocInfo, i);
	}
}
#pragma endregion

#pragma region RSRP_SINR_FOR_RRC
double LTENR_PHY_RRC_RSRP_SINR(NETSIM_ID gnbId, NETSIM_ID gnbIf,
							 NETSIM_ID ueId, NETSIM_ID ueIf) {
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_ALLOC();
	info->ueId = ueId;
	info->ueIf = ueIf;
	for (NETSIM_ID i = 0; i < phy->ca_count; i++) {
		if (!info->downlinkAMCInfo[i])
			info->downlinkAMCInfo[i] = calloc(1, sizeof * info->downlinkAMCInfo[i]);
		if (!info->uplinkAMCInfo[i])
			info->uplinkAMCInfo[i] = calloc(1, sizeof * info->uplinkAMCInfo[i]);

		LTENR_PHY_calculateSpectralEfficiency(phy, info, i);
	}
	double sinr = 0;
	for (NETSIM_ID i = 0; i < phy->ca_count; i++) {
		sinr += info->propagationInfo[i]->downlink.SNR_db;
	}
	for (NETSIM_ID i = 0; i < phy->ca_count; i++) {
		if (info->downlinkAMCInfo[i])
			free(info->downlinkAMCInfo[i]);
		if (info->uplinkAMCInfo[i])
			free(info->uplinkAMCInfo[i]);
		if (info->propagationInfo[i])
			free(info->propagationInfo[i]);
	}
	free(info);
	return sinr / phy->ca_count;
}
#pragma endregion


#pragma region PHY_ASSOCIATION
static void LTENR_PHY_associateUE(NETSIM_ID gnbId, NETSIM_ID gnbIf,
								  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	print_ltenr_log("PHY-- UE %d:%d is associated with gNB %d:%d\n",
					ueId, ueIf, gnbId, gnbIf);

	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_ALLOC();
	LTENR_ASSOCIATEDUEPHYINFO_ADD(phy, info);

	info->ueId = ueId;
	info->ueIf = ueIf;

	LTENR_PHY_initAMCInfo(phy, info);

	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(ueId, ueIf);
	uePhy->gnBId = gnbId;
	uePhy->gnbIf = gnbIf;

	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	LTENR_EPC_ASSOCIATION(mac->epcId, mac->epcIf,
						  gnbId, gnbIf,
						  ueId, ueIf);
}

static void LTENR_PHY_deassociateUE(NETSIM_ID gnbId, NETSIM_ID gnbIf,
								  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	while (info) {
		if (info->ueId == ueId && info->ueIf == ueIf) {
			LTENR_ASSOCIATEDUEPHYINFO_REMOVE(phy, info);
			break;
		}
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	}

}

void LTENR_PHY_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
						   NETSIM_ID ueId, NETSIM_ID ueIf,
						   bool isAssociated)
{
	if (isAssociated)
		LTENR_PHY_associateUE(gnbId, gnbIf, ueId, ueIf);
	else
		LTENR_PHY_deassociateUE(gnbId, gnbIf, ueId, ueIf);
}
#pragma endregion

#pragma region PHY_API
double LTENR_PHY_GetSlotEndTime(NETSIM_ID d, NETSIM_ID in)
{
	if (isGNB(d, in))
	{
		ptrLTENR_FRAMEINFO fi = ((ptrLTENR_GNBPHY)LTENR_GNBPHY_GET(d, in))->currentFrameInfo;
		return fi->slotEndTime;
	}
	else
	{
		ptrLTENR_UEPHY phy = LTENR_UEPHY_GET(d, in);
		return LTENR_PHY_GetSlotEndTime(phy->gnBId, phy->gnbIf);
	}
}

void fn_NetSim_UPDATE_SPECTRAL_EFFICIENCY_INFO(NETSIM_ID gnbID, NETSIM_ID gnbIF, 
	NETSIM_ID ueID, NETSIM_ID ueIF) {
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbID, gnbIF);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	while (info) {
		if (info->ueId == ueID && info->ueIf == ueIF) {
			NETSIM_ID i = 0;
			for (i = 0; i < phy->ca_count; i++) {
				LTENR_PHY_calculateSpectralEfficiency(phy, info, i);
			}
		}
		info = LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	}
}

#pragma endregion
