#pragma once
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
* Author:    Kumar Gaurav                                               *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_GNBRRC_H_
#define _NETSIM_LTENR_GNBRRC_H_
#include "LTENR_Msg.h"
#include "LTENR_PDCP.h"
#include "LTENR_RLC.h"

#ifdef  __cplusplus
extern "C" {
#endif

	//Default Timer
#define LTENR_MSG_MIB_REGENERATION_TIME		80*MILLISECOND
#define	LTENR_MSG_SIB1_REGENERATION_TIME    160*MILLISECOND
#define LTENR_MSG_UE_MEASUREMENT_REPORT_PERIOD_TIME 120*MILLISECOND
#define HANDOVER_DIFF 3//db

	//Default packet size
#define LTENR_MSG_MIB_SIZE					3	// Bytes

#pragma region RRC_STATE
	typedef enum enum_LTENR_UE_RRCSTATE
	{
		UERRC_IDLE,
		UERRC_INACTIVE,
		UERRC_CONNECTED,
	}LTENR_UE_RRCSTATE;
#pragma endregion

#pragma region MIB_MSG_Structure
	typedef struct stru_LTENR_RRC_PDCCH_ConfigSIB1
	{
		UINT ControlResourceSetZero : 4;//bits
		UINT SearchSpaceZero : 4;//bits
	}LTENR_RRC_PDCCH_ConfigSIB1, * ptrLTENR_RRC_PDCCH_ConfigSIB1;

	typedef enum enum_LTENR_RRC_subCarrierSpacingCommom {
		subCarrierSpacingCommon_scs15or60,
		subCarrierSpacingCommon_scs30or120,
	}LTENR_RRC_subCarrierSpacingCommon;
	static const char* strLTENR_RRC_subCarrierSpacingCommom[] = {
		"SCS15or60","SCS30or120"
	};

	typedef enum enum_LTENR_RRC_dmrsTypeAPosition {
		dmrsTypeAPosition_pos2,
		dmrsTypeAPosition_pos3,
	}LTENR_RRC_dmrsTypeAPosition;
	static const char* strLTENR_RRC_dmrsTypeAPosition[] = {
		"POS2","POS3"
	};

	typedef enum enum_LTENR_RRC_cellBarred {
		cellBarred_notBarred,
		cellBarred_barred,
	}LTENR_RRC_cellBarred;
	static const char* strLTENR_RRC_CellBarred[] = {
		"NotBarred","Barred"
	};

	typedef enum enum_LTENR_RRC_intraFreqReselection {
		intraFreqReselection_allowed,
		intraFreqReselection_notAllowed,
	}LTENR_RRC_intraFreqReselection;


	typedef struct stru_LTENR_MIBMSG {
		UINT systemFrameNumber;
		LTENR_RRC_subCarrierSpacingCommon subCarrierSpacingCommon;
		UINT ssbSubcarrierOffset;//used for FDD
		LTENR_RRC_dmrsTypeAPosition dmrsTypeAPosition;
		//ptrLTENR_RRC_PDCCH_ConfigSIB1 PDCCH_ConfigSIB1;
		LTENR_RRC_cellBarred cellBarred;
		LTENR_RRC_intraFreqReselection intraFreqReselection;
		bool spare;
	}LTENR_RRC_MIBMSG, * ptrLTENR_RRC_MIBMSG;

#pragma endregion

#pragma region SIB1_MSG_Structure
	typedef struct stru_LTENR_RRC_QRxLevMin {
		int Q_RxLevMin;
	}LTENR_RRC_QRxLevMin, * ptrLTENR_RRC_QRxLevMin;

	typedef struct stru_LTENR_RRC_CellSelectionInfo {
		int q_RxLevMin;
		UINT q_RxLevMinOffset : 4;
		int q_RxLevMinSul;
	}LTENR_RRC_CellSelectionInfo, * ptrLTENR_RRC_CellSelectionInfo;

	typedef struct stru_LTENR_RRC_MCC_MNC_DIGIT {
		UINT mccMNCDigit : 4;
	}stru_LTENR_RRC_MCC_MNC_DIGIT, * ptrstru_LTENR_RRC_MCC_MNC_DIGIT;

	typedef struct stru_LTENR_RRC_PLMNIdentity {
		ptrstru_LTENR_RRC_MCC_MNC_DIGIT MCC;
		ptrstru_LTENR_RRC_MCC_MNC_DIGIT MNC;
	}LTENR_RRC_PLMNIdentity, * ptrLTENR_RRC_PLMNIdentity;

	typedef struct stru_LTENR_RRC_RANAreaCode {
		UINT ranAreaCode : 4;
	}LTENR_RRC_RANAreaCode, * ptrLTENR_RRC_RANAreaCode;

	typedef struct stru_LTENR_RRC_CellIdentity {
		int cellIdentity;
	}LTENR_RRC_CellIdentity, * ptrLTENR_RRC_CellIdentity;

	typedef enum enum_LTENR_RRC_CellReservedForOperatorUse {
		CellReservedForOperatorUse_reserved,
		CellReservedForOperatorUse_notReserved,
	}LTENR_RRC_CellReservedForOperatorUse;
	static const char* strLTENR_RRC_CellReservedForOperatorUse[] = {
		"CellReservedForOperatorUse_reserved","CellReservedForOperatorUse_notReserved"
	};
	typedef struct stru_LTENR_RRC_PLMNIdentityInfo {
		ptrLTENR_RRC_PLMNIdentity PLMNIedentity;
		int trackingAreaCode;
		ptrLTENR_RRC_RANAreaCode ranAreaCode;
		UINT cellIdentity;
		LTENR_RRC_CellReservedForOperatorUse cellReservedForOperatorUse;

	}LTENR_RRC_PLMNIdentityInfo, * ptrLTENR_RRC_PLMNIdentityInfo;

	typedef struct stru_LTENR_RRC_CellAccessRelatedInfo {
		ptrLTENR_RRC_PLMNIdentityInfo plmnIdentityInfo;//max-PLMN = 12
		bool cellReservedForOtherUse;//always true
	}LTENR_RRC_CellAccessRelatedInfo, * ptrLTENR_RRC_CellAccessRelatedInfo;

	typedef enum enum_LTENR_RRC_ConnEstFailCount {
		ConnEstFailCount_n1,
		ConnEstFailCount_n2,
		ConnEstFailCount_n3,
		ConnEstFailCount_n4,
	}LTENR_RRC_ConnEstFailCount;
	static const char* strLTENR_RRC_ConnEstFailCount[] = {
		"n1","n2","n3","n4"
	};
	typedef enum enum_LTENR_RRC_ConnEstFailOffsetValidity {
		ConnEstFailOffsetValidity_s30,
		ConnEstFailOffsetValidity_s60,
		ConnEstFailOffsetValidity_s120,
		ConnEstFailOffsetValidity_s240,
		ConnEstFailOffsetValidity_s300,
		ConnEstFailOffsetValidity_s420,
		ConnEstFailOffsetValidity_s600,
		ConnEstFailOffsetValidity_s900,
	}LTENR_RRC_ConnEstFailOffsetValidity;
	static const char* strLTENR_RRC_ConnEstFailOffsetValidity[] = {
		"30s","60s","120s","240s","300s","420s","600s","900s"
	};
	typedef struct stru_LTENR_RRC_ConnEstFailureControl {
		LTENR_RRC_ConnEstFailCount connEstFailCount;
		LTENR_RRC_ConnEstFailOffsetValidity connEstFailOffsetValidity;
		UINT connEstFailOffset : 4;//optional
	}LTENR_RRC_ConnEstFailureControl, * ptrLTENR_RRC_ConnEstFailureControl;

	typedef enum enum_LTENR_RRC_SIWindowLength {
		SIWindowLength_s5,
		SIWindowLength_s10,
		SIWindowLength_s20,
		SIWindowLength_s40,
		SIWindowLength_s80,
		SIWindowLength_s160,
		SIWindowLength_s320,
		SIWindowLength_s640,
		SIWindowLength_s1280,
	}LTENR_RRC_SIWindowLength;

	typedef enum enum_LTENR_RRC_SIBroadcastStatus {
		SIBroadcastStatus_broadcasting,
		SIBroadcastStatus_notBroadcasting,
	}LTENR_RRC_SIBroadcastStatus;

	typedef enum enum_LTENR_RRC_SIPeriodicity {
		SIPeriodicity_rf8,
		SIPeriodicity_rf16,
		SIPeriodicity_rf32,
		SIPeriodicity_rf64,
		SIPeriodicity_rf128,
		SIPeriodicity_rf256,
		SIPeriodicity_rf512,
	}LTENR_RRC_SIPeriodicity;

	typedef enum enum_LTENR_RRC_SIBType {
		sibType2, sibType3, sibType4, sibType5, sibType6, sibType7, sibType8, sibType9,
		SIBType_spare8, SIBType_spare7, SIBType_spare6, SIBType_spare5, SIBType_spare4,
		SIBType_spare3, SIBType_spare2, SIBType_spare1,
	}LTENR_RRC_SIBType;

	typedef struct stru_LTENR_RRC_SIBTypeInfo {
		LTENR_RRC_SIBType type;
		UINT valueTag : 4;
		bool areaScope;//always true
	}LTENR_RRC_SIBTypeInfo, * ptrLTENR_RRC_SIBTypeInfo;

	typedef struct stru_LTENR_RRC_SIBMapping {
		ptrLTENR_RRC_SIBTypeInfo sibTypeInfo;
	}LTENR_RRC_SIBMapping, * ptrLTENR_RRC_SIBMapping;

	typedef struct stru_LTENR_RRC_SchedulingInfo {
		LTENR_RRC_SIBroadcastStatus siBroadcastStatus;
		LTENR_RRC_SIPeriodicity siPeriodicity;
		ptrLTENR_RRC_SIBMapping sibMapping;

		_ptr_ele ele;//list
	}LTENR_RRC_SchedulingInfo, * ptrLTENR_RRC_SchedulingInfo;

	typedef enum enum_LTENR_RRC_SIRequestPeriod {
		SIRequestPeriod_one = 1,
		SIRequestPeriod_two = 2,
		SIRequestPeriod_four = 4,
		SIRequestPeriod_six = 6,
		SIRequestPeriod_eight = 8,
		SIRequestPeriod_ten = 10,
		SIRequestPeriod_twelve = 12,
		SIRequestPeriod_sixteen = 16,
	}LTENR_RRC_SIRequestPeriod;

	typedef enum enum_LTENR_RRC_MSG1FDM {
		LTENR_RRC_MSG1FDM_one = 1,
		LTENR_RRC_MSG1FDM_two = 2,
		LTENR_RRC_MSG1FDM_four = 4,
		LTENR_RRC_MSG1FDM_eight = 8,
	}LTENR_RRC_MSG1FDM;

	typedef enum enum_LTENR_RRC_PreambleTransMax {
		PreambleTransMax_n3,
		PreambleTransMax_n4,
		PreambleTransMax_n5,
		PreambleTransMax_n6,
		PreambleTransMax_n7,
		PreambleTransMax_n8,
		PreambleTransMax_n10,
		PreambleTransMax_n20,
		PreambleTransMax_n50,
		PreambleTransMax_n100,
		PreambleTransMax_n200,
	}LTENR_RRC_PreambleTransMax;

	typedef enum enum_LTENR_RRC_PowerRampingStep {
		PowerRampingStep_dB0 = 0,
		PowerRampingStep_dB2 = 2,
		PowerRampingStep_dB4 = 4,
		PowerRampingStep_dB6 = 6,
	}LTENR_RRC_PowerRampingStep;

	typedef enum enum_LTENR_RRC_RaResponseWindow {
		RaResponseWindow_sl1,
		RaResponseWindow_sl2,
		RaResponseWindow_sl4,
		RaResponseWindow_sl8,
		RaResponseWindow_sl10,
		RaResponseWindow_sl20,
		RaResponseWindow_sl40,
		RaResponseWindow_sl80
	}LTENR_RRC_RaResponseWindow;

	typedef struct stru_LTENR_RRC_RACHConfigGeneric {
		UINT prachConfigIndex : 4;
		LTENR_RRC_MSG1FDM msg1FDM;
		UINT msg1FrequencyStart : 4;
		UINT zeroCorrelationZoneConfig : 4;
		UINT preambleReceivedTargetPower : 4;
		LTENR_RRC_PreambleTransMax preambleTransMax;
		LTENR_RRC_PowerRampingStep powerRampingStep;
		LTENR_RRC_RaResponseWindow raResponseWindow;
	}LTENR_RRC_RACHConfigGeneric, * ptrLTENR_RRC_RACHConfigGeneric;

	typedef struct stru_LTENR_RRC_RachOccasionSI {
		ptrLTENR_RRC_RACHConfigGeneric rachConfigGeneric;
		LTENR_RRC_SIRequestPeriod siRequestPeriod;
	}LTENR_RRC_RachOccasionSI, * ptrLTENR_RRC_RachOccasionSI;

	typedef struct struc_LTENR_RRC_SIRequestResource {
		UINT raPreambleStartIndex : 4;
		UINT raAssociationPeriodIndex : 4;
		UINT rassbOccasionMaskIndex : 4;
	}LTENR_RRC_SIRequestResource, * ptrLTENR_RRC_SIRequestResource;

	typedef struct stru_LTENR_RRC_SIRequestConfig {
		ptrLTENR_RRC_RachOccasionSI rachOccasionSI;
		LTENR_RRC_SIRequestPeriod siRequestPeriod;
		ptrLTENR_RRC_SIRequestResource siRequestResource;
	}LTENR_RRC_SIRequestConfig, * ptrLTENR_RRC_SIRequestConfig;

	typedef struct stru_LTENR_RRC_SISchedulingInfo {
		ptrLTENR_RRC_SchedulingInfo schedulingInfo[32];//max SIMSG info is 32
		LTENR_RRC_SIWindowLength siWindowLength;
		ptrLTENR_RRC_SIRequestConfig siRequestConfig;
		ptrLTENR_RRC_SIRequestConfig siReuestConfigSUL;
		UINT systemInformationAreaID : 24;
	}LTENR_RRC_SISchedulingInfo, * ptrLTENR_RRC_SISchedulingInfo;

	typedef struct stru_LTENR_RRC_PMax {
		UINT PMax : 4;
	}LTENR_RRC_PMax, * ptrLTENR_RRC_PMax;

	typedef struct stru_LTENR_RRC_AdditionalSpectrumEmission {
		UINT additionalSpectrumEmission : 4;
	}LTENR_RRC_AdditionalSpectrumEmission, * ptrLTENR_RRC_AdditionalSpectrumEmission;

	typedef struct stru_LTENR_RRC_NRNSPmaxValue {
		ptrLTENR_RRC_PMax additionalPMax;
		ptrLTENR_RRC_AdditionalSpectrumEmission additionalSpectrumEmission;
	}LTENR_RRC_NRNSPmaxValue, * ptrLTENR_RRC_NRNSPmaxValue;

	typedef struct stru_LTENR_RRC_FreqBandIndicationNR {
		UINT freqBandIndicatorNR : 4;
	}LTENR_RRC_FreqBandIndicationNR, * ptrLTENR_RRC_FreqBandIndicationNR;

	typedef struct stru_LTENR_RRC_NRMultiBandInfo {
		ptrLTENR_RRC_FreqBandIndicationNR FreqBandIndicatorNR;
		ptrLTENR_RRC_NRNSPmaxValue NRNSPMax;
	}TENR_RRC_NRMultiBandInfo, * ptrTENR_RRC_NRMultiBandInfo;

	typedef enum enum_LTENR_RRC_SubCarrierSpacing {
		SubCarrierSpacing_kHz15,
		SubCarrierSpacing_kHz30,
		SubCarrierSpacing_kHz60,
		SubCarrierSpacing_kHz120,
		SubCarrierSpacing_kHz240,
		SubCarrierSpacing_spare3,
		SubCarrierSpacing_spare2,
		SubCarrierSpacing_spare1,
	}LTENR_RRC_SubCarrierSpacing;

	typedef struct stru_LTENR_RRC_SCSSpecificCarrier {
		UINT offsetToCarrier : 4;
		LTENR_RRC_SubCarrierSpacing subCarrierSpacing;
		UINT carrierBandwidth : 4;
	}LTENR_RRC_SCSSpecificCarrier, * ptrLTENR_RRC_SCSSpecificCarrier;

	typedef struct stru_LTENR_RRC_FrequencyInfoDL_SIB {
		ptrTENR_RRC_NRMultiBandInfo MultiFrequencyBand;//list want debugging ask sir
		UINT offsetToPointA : 4;
		ptrLTENR_RRC_SCSSpecificCarrier sscSpecificCarrier;//list want debugging ask sir
	}LTENR_RRC_FrequencyInfoDL_SIB, * ptrLTENR_RRC_FrequencyInfoDL_SIB;

	typedef enum enum_LTENR_RRC_CyclicPrefix {
		CyclicPrefix_extended,
	}LTENR_RRC_CyclicPrefix;

	typedef struct stru_LTENR_RRC_BWP {
		UINT locationAndBandwidth : 4;
		LTENR_RRC_SubCarrierSpacing subcarrierSpacing;
		LTENR_RRC_CyclicPrefix cyclicPrefix;
	}LTENR_RRC_BWP, * ptrLTENR_RRC_BWP;

	typedef struct stru_LTENR_RRC_BWP_DownLinkCommon {
		ptrLTENR_RRC_BWP BWP;
		//pDCCH and PDSCH
	}LTENR_RRC_BWP_DownLinkCommon, * ptrLTENR_RRC_BWP_DownLinkCommon;

	typedef enum enum_LTENR_RRC_BCCH_Config_modificationPeriodCoeff {
		Config_modificationPeriodCoeff_n2,
		Config_modificationPeriodCoeff_n4,
		Config_modificationPeriodCoeff_n8,
		Config_modificationPeriodCoeff_n16,
	}LTENR_RRC_BCCH_Config_modificationPeriodCoeff;

	typedef struct stru_LTENR_RRC_BCCH_Config {
		LTENR_RRC_BCCH_Config_modificationPeriodCoeff modificationPeriodCoeff;
	}LTENR_RRC_BCCH_Config, * ptrLTENR_RRC_BCCH_Config;

	typedef enum enum_LTENR_RRC_pagingCycle {
		pagingCycle_rf32,
		pagingCycle_rf64,
		pagingCycle_rf128,
		pagingCycle_rf256,
	}LTENR_RRC_pagingCycle;

	typedef enum enum_LTENR_RRC_ns {
		ns_four = 4,
		ns_two = 2,
		ns_one = 1,
	}LTENR_RRC_ns;


	typedef struct stru_LTENR_RRC_PCCH_Config {
		LTENR_RRC_pagingCycle defaultPagingCycle;
		//this make it fully structure not fully written
		LTENR_RRC_ns ns;
	}LTENR_RRC_PCCH_Config, * ptrLTENR_RRC_PCCH_Config;

	typedef struct stru_LTENR_RRC_DownlinConfigCommonSIB {
		ptrLTENR_RRC_FrequencyInfoDL_SIB frequencyInfoDL;
		ptrLTENR_RRC_BWP_DownLinkCommon initialDownLinkBWP;
		ptrLTENR_RRC_BCCH_Config BCCH_Config;
		ptrLTENR_RRC_PCCH_Config PCCH_Config;
	}LTENR_RRC_DownlinConfigCommonSIB, * ptrLTENR_RRC_DownlinConfigCommonSIB;

	typedef enum enum_LTENR_RRC_NTimingAdvanceOffset {
		NTimingAdvanceOffset_n0,
		NTimingAdvanceOffset_n25600,
		NTimingAdvanceOffset_n39936,
	}LTENR_RRC_NTimingAdvanceOffset;

	typedef enum enum_LTENR_RRC_SSBPeriodicityServingCell {
		SSBPeriodicityServingCell_ms5,
		SSBPeriodicityServingCell_ms10,
		SSBPeriodicityServingCell_ms20,
		SSBPeriodicityServingCell_ms40,
		SSBPeriodicityServingCell_ms80,
		SSBPeriodicityServingCell_ms160,
		SSBPeriodicityServingCell_spare2,
		SSBPeriodicityServingCell_spare1,
	}LTENR_RRC_SSBPeriodicityServingCell;

	typedef struct stru_LTENR_RRC_SSBPositionsInBrust {
		char shortBitMap[4];
		char mediumBitMap[8];
		char longBitMap[64];
	}LTENR_RRC_SSBPositionsInBrust, * ptrLTENR_RRC_SSBPositionsInBrust;

	typedef enum enum_LTENR_RRC_T300 {
		T300_ms100,
		T300_ms200,
		T300_ms300,
		T300_ms400,
		T300_ms600,
		T300_ms1000,
		T300_ms1500,
		T300_ms2000,
	}LTENR_RRC_T300;
	static const char* strLTENR_RRC_T300[] = {
		"100ms","200ms","300ms","400ms","600ms","1000ms","1500ms","2000ms"
	};
	typedef enum enum_LTENR_RRC_T301 {
		T301_ms100,
		T301_ms200,
		T301_ms300,
		T301_ms400,
		T301_ms600,
		T301_ms1000,
		T301_ms1500,
		T301_ms2000,
	}LTENR_RRC_T301;
	static const char* strLTENR_RRC_T301[] = {
		"100ms","200ms","300ms","400ms","600ms","1000ms","1500ms","2000ms"
	};
	typedef enum enum_LTENR_RRC_T310 {
		T310_ms0,
		T310_ms50,
		T310_ms100,
		T310_ms200,
		T310_ms500,
		T310_ms1000,
		T310_ms2000,
	}LTENR_RRC_T310;
	static const char* strLTENR_RRC_T310[] = {
		"0ms","50ms","100ms","200ms","500ms","1000ms","2000ms"
	};
	typedef enum enum_LTENR_RRC_N310 {
		N310_n1,
		N310_n2,
		N310_n3,
		N310_n4,
		N310_n6,
		N310_n8,
		N310_n10,
		N310_n20,
	}LTENR_RRC_N310;

	typedef enum enum_LTENR_RRC_T311 {
		T311_ms1000,
		T311_ms3000,
		T311_ms5000,
		T311_ms10000,
		T311_ms15000,
		T311_ms20000,
		T311_ms30000,
	}LTENR_RRC_T311;
	static const char* strLTENR_RRC_T311[] = {
		"1000ms","3000ms","5000ms","10000ms","0","20000ms","30000ms"
	};
	typedef enum enum_LTENR_RRC_N311 {
		N311_n1,
		N311_n2,
		N311_n3,
		N311_n4,
		N311_n5,
		N311_n6,
		N311_n8,
		N311_n10,
	}LTENR_RRC_N311;

	typedef enum enum_LTENR_RRC_T319 {
		T319_ms100,
		T319_ms200,
		T319_ms300,
		T319_ms400,
		T319_ms600,
		T319_ms1000,
		T319_ms1500,
		T319_ms2000,
	}LTENR_RRC_T319;
	static const char* strLTENR_RRC_T319[] = {
		"100ms","200ms","300ms","400ms","600ms","1000ms","1500ms","2000ms"
	};
	typedef struct stru_LTENR_RRC_UETimersAndConstants {
		LTENR_RRC_T300 t300;
		LTENR_RRC_T301 t301;
		LTENR_RRC_T310 t310;
		LTENR_RRC_N310 n310;
		LTENR_RRC_T311 t311;
		LTENR_RRC_N311 n311;
		LTENR_RRC_T319 t319;
	}LTENR_RRC_UETimersAndConstants, * ptrLTENR_RRC_UETimersAndConstants;

	typedef struct stru_LTENR_RRC_ServingCellConfigCommonSIB {
		ptrLTENR_RRC_DownlinConfigCommonSIB downLinkConfigCommon;
		LTENR_RRC_NTimingAdvanceOffset nTimingAdvanceOffset;
		ptrLTENR_RRC_SSBPositionsInBrust ssbPositionsInBrust;
		LTENR_RRC_SSBPeriodicityServingCell ssbPeriodicityServingCell;
		LTENR_RRC_SubCarrierSpacing subCarrierSpacing;
	}LTENR_RRC_ServingCellConfigCommonSIB, * ptrLTENR_RRC_ServingCellConfigCommonSIB;

	typedef struct stru_LTENR_RRC_UACBarringInfoSetIndex {
		UINT uacBarringInfoSetIndex : 4;
	}LTENR_RRC_UACBarringInfoSetIndex, * ptrLTENR_RRC_UACBarringInfoSetIndex;

	typedef struct stru_LTENR_RRC_UACBarringPerCat {
		UINT accessCategory : 4;
		ptrLTENR_RRC_UACBarringInfoSetIndex uacBarringInfoSetIndex;
	}LTENR_RRC_UACBarringPerCat, * ptrLTENR_RRC_UACBarringPerCat;

	typedef struct stru_LTENR_UACACBarringListType {
		ptrLTENR_RRC_UACBarringInfoSetIndex uacImplicitACBarringList;
		ptrLTENR_RRC_UACBarringPerCat uacExplicitACBarringList;
	}LTENR_UACACBarringListType, * ptrLTENR_UACACBarringListType;

	typedef struct stru_LTENR_RRC_UACBarringPerPLMN {
		UINT plmnIdentityIndex : 4;
		ptrLTENR_UACACBarringListType uacBarringListType;
	}LTENR_RRC_UACBarringPerPLMN, * ptrLTENR_RRC_UACBarringPerPLMN;

	typedef enum enum_LTENR_RRC_UACBarringFactor {
		UACBarringFactor_p00,
		UACBarringFactor_p05,
		UACBarringFactor_p10,
		UACBarringFactor_p15,
		UACBarringFactor_p20,
		UACBarringFactor_p25,
		UACBarringFactor_p30,
		UACBarringFactor_p40,
		UACBarringFactor_p50,
		UACBarringFactor_p60,
		UACBarringFactor_p70,
		UACBarringFactor_p75,
		UACBarringFactor_p80,
		UACBarringFactor_p85,
		UACBarringFactor_p90,
		UACBarringFactor_p95,
	}LTENR_RRC_UACBarringFactor;

	typedef enum enum_LTENR_RRC_UACBarringTimer {
		UACBarringTime_s4,
		UACBarringTime_s8,
		UACBarringTime_s16,
		UACBarringTime_s32,
		UACBarringTime_s64,
		UACBarringTime_s128,
		UACBarringTime_s256,
		UACBarringTime_s512,
	}LTENR_RRC_UACBarringTime;

	typedef struct stru_LTENR_RRC_UACBarringInfoSet {
		LTENR_RRC_UACBarringFactor uacBarringFactor;
		LTENR_RRC_UACBarringTime uacBarringTime;
		char uacBarringForAAccessIdentity[7];
	}LTENR_RRC_UACBarringInfoSet, * ptrLTENR_RRC_UACBarringInfoSet;

	typedef enum enum_LTENR_RRC_UACAccessCategory1SelectionAssistanceInfo {
		UACAccessCategory1SelectionAssistanceInfo_a,
		UACAccessCategory1SelectionAssistanceInfo_b,
		UACAccessCategory1SelectionAssistanceInfo_c,
	}LTENR_RRC_UACAccessCategory1SelectionAssistanceInfo;

	typedef struct stru_LTENR_RRC_UACAccessCategory1SelectionAssistanceInfoChoice {
		LTENR_RRC_UACAccessCategory1SelectionAssistanceInfo plmnCommon;
	}LTENR_RRC_UACAccessCategory1SelectionAssistanceInfoChoice, * ptrLTENR_RRC_UACAccessCategory1SelectionAssistanceInfoChoice;

	typedef struct stru_LTENR_RRC_UACBarringInfo {
		ptrLTENR_RRC_UACBarringPerCat uacBarringForCommon;
		ptrLTENR_RRC_UACBarringPerPLMN uacBarringPerPLMN;
		ptrLTENR_RRC_UACBarringInfoSet uacBArringInfoSet;
		ptrLTENR_RRC_UACAccessCategory1SelectionAssistanceInfoChoice uacAccessCategory1SelectionAssistanceInfoChoice;
	}LTENR_RRC_UACBarringInfo, * ptrLTENR_RRC_UACBarringInfo;

	typedef struct stru_LTENR_SIB1MSG {
		ptrLTENR_RRC_CellSelectionInfo cellSelectionInfo;
		ptrLTENR_RRC_CellAccessRelatedInfo cellAccessRelatedInfo;
		ptrLTENR_RRC_ConnEstFailureControl cellEstFailureControl;
		//ptrLTENR_RRC_SISchedulingInfo siSchedulingInfo;
		//ptrLTENR_RRC_ServingCellConfigCommonSIB servingCellConfigCommon;
		bool imsEmergencySupport;//always true
		bool ecallOverIMSSupport;//always true
		ptrLTENR_RRC_UETimersAndConstants ueTimersandConstants;
		//ptrLTENR_RRC_UACBarringInfo uacBarringInfo; 
		bool useFullResumeHD;//always true
		double SINR;
		double RSRP;
		double RSRQ;
	}LTENR_RRC_SIB1MSG, * ptrLTENR_RRC_SIB1MSG;

#pragma endregion

#pragma region SIB2_MSG_Structure
	typedef struct stru_LTENR_RRC_RSRPRange {
		UINT rsrpRange : 4;
	}LTENR_RRC_RSRPRange, * ptrLTENR_RRC_RSRPRange;

	typedef struct stru_LTENR_RRC_RSRQRange {
		UINT rsrqRange : 4;
	}LTENR_RRC_RSRQRange, * ptrLTENR_RRC_RSRQRange;

	typedef struct stru_LTENR_RRC_SINRRange {
		UINT sinrRange : 4;
	}LTENR_RRC_SINRRange, * ptrLTENR_RRC_SINRRange;

	typedef enum enum_LTENR_RRC_QOffsetRange {
		QOffsetRange_dB_24,
		QOffsetRange_dB_22,
		QOffsetRange_dB_20,
		QOffsetRange_dB_18,
		QOffsetRange_dB_16,
		QOffsetRange_dB_14,
		QOffsetRange_dB_12,
		QOffsetRange_dB_10,
		QOffsetRange_dB_8,
		QOffsetRange_dB_6,
		QOffsetRange_dB_5,
		QOffsetRange_dB_4,
		QOffsetRange_dB_3,
		QOffsetRange_dB_2,
		QOffsetRange_dB_1,
		QOffsetRange_dB0,
		QOffsetRange_dB1,
		QOffsetRange_dB2,
		QOffsetRange_dB3,
		QOffsetRange_dB4,
		QOffsetRange_dB5,
		QOffsetRange_dB6,
		QOffsetRange_dB8,
		QOffsetRange_dB10,
		QOffsetRange_dB12,
		QOffsetRange_dB14,
		QOffsetRange_dB16,
		QOffsetRange_dB18,
		QOffsetRange_dB20,
		QOffsetRange_dB22,
		QOffsetRange_dB24,
	}LTENR_RRC_QOffsetRange;

	typedef enum enum_LTENR_RRC_TEvalution {
		TEvalution_s30,
		TEvalution_s60,
		TEvalution_s120,
		TEvalution_s180,
		TEvalution_s240,
		TEvalution_spare3,
		TEvalution_spare2,
		TEvalution_spare1,
	}LTENR_RRC_TEvalution;

	typedef enum enum_LTENR_RRC_THystNormal {
		THystNormal_s30,
		THystNormal_s60,
		THystNormal_s120,
		THystNormal_s180,
		THystNormal_s240,
		THystNormal_spare3,
		THystNormal_spare2,
		THystNormal_spare1,
	}LTENR_RRC_THystNormal;

	typedef enum enum_LTENR_RRC_SFMedium {
		SFMedium_dB_6,
		SFMedium_dB_4,
		SFMedium_dB_2,
		SFMedium_dB0,
	}LTENR_RRC_SFMedium;

	typedef enum enum_LTENR_RRC_SFHigh {
		SFHigh_dB_6,
		SFHigh_dB_4,
		SFHigh_dB_2,
		SFHigh_dB0,
	}LTENR_RRC_SFHigh;

	typedef struct stru_LTENR_RRC_ReselectionThreshold {
		UINT reReselectionThreshold : 4;
	}LTENR_RRC_ReselectionThreshold, * ptrLTENR_RRC_ReselectionThreshold;

	typedef struct stru_LTENR_RRC_ReselectionThresholdQ {
		UINT reReselectionThresholdQ : 4;
	}LTENR_RRC_ReselectionThresholdQ, * ptrLTENR_RRC_ReselectionThresholdQ;

	typedef struct stru_LTENR_RRC_TReselection {
		UINT tReselection : 4;
	}LTENR_RRC_TReselection, * ptrLTENR_RRC_TReselection;

	typedef struct stru_LTENR_RRC_CellReselectionPriority {
		UINT cellReselectionPriority : 4;
	}LTENR_RRC_CellReselectionPriority, * ptrLTENR_RRC_CellReselectionPriority;

	typedef struct stru_LTENR_RRC_QHystSF {
		LTENR_RRC_SFMedium sfMedium;
		LTENR_RRC_SFHigh sfHigh;
	}LTENR_RRC_QHystSF, * ptrLTENR_RRC_QHystSF;

	typedef struct stru_LTENR_RRC_MobilityStateParameters {
		LTENR_RRC_TEvalution tEvaluation;
		LTENR_RRC_THystNormal tHystNormal;
		UINT nCellChanngeMedium : 4;
		UINT nCellChangeHigh : 4;
	}LTENR_RRC_MobilityStateParameters, * ptrLTENR_RRC_MobilityStateParameters;

	typedef struct stru_LTENR_RRC_SpeedStateReselectionPars {
		ptrLTENR_RRC_MobilityStateParameters mobilityStateParameters;
		ptrLTENR_RRC_QHystSF qHystsSF;
	}LTENR_RRC_SpeedStateReselectionPars, * ptrLTENR_RRC_SpeedStateReselectionPars;

	typedef enum enum_LTENR_RRC_QHyst {
		QHyst_dB0,
		QHyst_dB1,
		QHyst_dB2,
		QHyst_dB3,
		QHyst_dB4,
		QHyst_dB5,
		QHyst_dB6,
		QHyst_dB8,
		QHyst_dB10,
		QHyst_dB12,
		QHyst_dB14,
		QHyst_dB16,
		QHyst_dB18,
		QHyst_dB20,
		QHyst_dB22,
		QHyst_dB24,
	}LTENR_RRC_QHyst;

	typedef struct stru_LTENR_RRC_RangeToBestCell {
		LTENR_RRC_QOffsetRange qOffsetRange;
	}LTENR_RRC_RangeToBestCell, * ptrLTENR_RRC_RangeToBestCell;

	typedef struct stru_LTENR_RRC_ThresholdNR {
		ptrLTENR_RRC_RSRPRange rsrpRange;
		ptrLTENR_RRC_RSRQRange rsrqRange;
		ptrLTENR_RRC_SINRRange sinrRange;
	}LTENR_RRC_ThresholdNR, * ptrLTENR_RRC_ThresholdNR;

	typedef struct stru_LTENR_RRC_PeriodicityAndOffset {
		UINT sf5 : 4;
		UINT sf10 : 4;
		UINT sf20 : 4;
		UINT sf40 : 4;
		UINT sf80 : 4;
		UINT sf160 : 4;
	}LTENR_RRC_PeriodicityAndOffset, * ptrLTENR_RRC_PeriodicityAndOffset;

	typedef struct stru_LTENR_RRC_SSBToMeasure {
		char shortBitMap[4];
		char mediumBitMap[8];
		char longBitMap[64];
	}LTENR_RRC_SSBToMeasure, * ptrLTENR_RRC_SSBToMeasure;

	typedef enum enum_LTENR_RRC_Duration {
		Duration_sf1,
		Duration_sf2,
		Duration_sf3,
		Duration_sf4,
		Duration_sf5,
	}LTENR_RRC_Duration;

	typedef enum enum_LTENR_RRC_SFMediumSpeedState {
		SFMediumSpeedState_oDot25,
		SFMediumSpeedState_oDot5,
		SFMediumSpeedState_oDot75,
		SFMediumSpeedState_lDot0,
	}LTENR_RRC_SFMediumSpeedState;

	typedef enum enum_LTENR_RRC_SFHighSpeedState {
		SFHighSpeedState_oDot25,
		SFHighSpeedState_oDot5,
		SFHighSpeedState_oDot75,
		SFHighSpeedState_lDot0,
	}LTENR_RRC_SFHighSpeedState;

	typedef struct stru_LTENR_RRC_SpeedStateScaleFactors {
		LTENR_RRC_SFMediumSpeedState sfMedium;
		LTENR_RRC_SFHighSpeedState sfHigh;
	}LTENR_RRC_SpeedStateScaleFactors, * ptrLTENR_RRC_SpeedStateScaleFactors;

	typedef struct stru_LTENR_RRC_SSRSSIMeasurement {
		char measurementSlots[80];
		UINT endSymbol : 4;
	}LTENR_RRC_SSRSSIMeasurement, * ptrLTENR_RRC_SSRSSIMeasurement;

	typedef struct stru_LTENR_RRC_SSBMTC {
		ptrLTENR_RRC_PeriodicityAndOffset periodicityandOffset;
		LTENR_RRC_Duration duration;
	}LTENR_RRC_SSBMTC, * ptrLTENR_RRC_SSBMTC;

	typedef struct stru_LTENR_RRC_CellReselectionInfoCommon {
		UINT nrofSSBlockToAverage : 4;
		ptrLTENR_RRC_ThresholdNR absThreshSSBlockConsolidation;
		ptrLTENR_RRC_RangeToBestCell rangeToBestCell;
		LTENR_RRC_QHyst qHyst;
		ptrLTENR_RRC_SpeedStateReselectionPars sppedStateReselectionPars;
	}LTENR_RRC_CellReselectionInfoCommon, * ptrLTENR_RRC_CellReselectionInfoCommon;

	typedef enum enum_LTENR_RRC_CellReselectionSubPriority {
		LTENR_RRC_CellReselectionSubPriority_oDot2,
		LTENR_RRC_CellReselectionSubPriority_oDot4,
		LTENR_RRC_CellReselectionSubPriority_oDot6,
		LTENR_RRC_CellReselectionSubPriority_oDot8,
	}LTENR_RRC_CellReselectionSubPriority;

	typedef struct stru_LTENR_RRC_CellReselectionServingFreqInfo {
		ptrLTENR_RRC_ReselectionThreshold sNonIntraSearchP;
		ptrLTENR_RRC_ReselectionThresholdQ sNonIntraSearchQ;
		ptrLTENR_RRC_ReselectionThreshold threshServingLowP;
		ptrLTENR_RRC_ReselectionThresholdQ threshServingLowQ;
		ptrLTENR_RRC_CellReselectionPriority  cellReselectionPriority;
		LTENR_RRC_CellReselectionSubPriority cellReselectionSubPriority;
	}LTENR_RRC_CellReselectionServingFreqInfo, * ptrLTENR_RRC_CellReselectionServingFreqInfo;

	typedef struct stru_LTENR_RRC_IntraFrqCellReselectionInfo {
		ptrLTENR_RRC_QRxLevMin qRxLevMin;
		ptrLTENR_RRC_QRxLevMin qRxLevMinSUL;
		int qQualMin;
		ptrLTENR_RRC_ReselectionThreshold sIntraSearchP;
		ptrLTENR_RRC_ReselectionThresholdQ tIntraSearchQ;
		ptrLTENR_RRC_TReselection tReselection;
		ptrTENR_RRC_NRMultiBandInfo freqBand;//Check list
		ptrTENR_RRC_NRMultiBandInfo freqBandSUL;
		ptrLTENR_RRC_PMax pMax;
		ptrLTENR_RRC_SSBMTC smtc;
		ptrLTENR_RRC_SSRSSIMeasurement ssRSSIMeasurement;
		ptrLTENR_RRC_SSBToMeasure ssbToMeasure;
		bool deriveSSBIndexFromCell;
		ptrLTENR_RRC_SpeedStateScaleFactors tReselectionNRSF;
	}LTENR_RRC_IntraFrqCellReselectionInfo, * ptrLTENR_RRC_IntraFreqCellReselectionInfo;

	typedef struct stru_LTENR_RRC_SIB2MSG {
		ptrLTENR_RRC_CellReselectionInfoCommon cellReselectionCommon;
		ptrLTENR_RRC_CellReselectionServingFreqInfo cellReselectionServingFrqInfo;
		ptrLTENR_RRC_IntraFreqCellReselectionInfo intraFreqCellReselectionInfo;
	}LTENR_RRC_SIB2MSG, * ptrLTENR_RRC_SIB2MSG;

#pragma endregion

#pragma region SIB3_MSG_Structure
	typedef struct stru_LTENR_RRC_PhysCellID {
		UINT physCellId : 4;
	}LTENR_RRC_PhysCellID, * ptrLTENR_RRC_PhysCellID;

	typedef enum enum_LTENR_RRC_Range {
		Range_n4,
		Range_n8,
		Range_n12,
		Range_n16,
		Range_n24,
		Range_n32,
		Range_n48,
		Range_n64,
		Range_n84,
		Range_n96,
		Range_n128,
		Range_n168,
		Range_n252,
		Range_n504,
		Range_n1008,
		Range_spare1,
	}LTENR_RRC_Range;

	typedef struct stru_LTENR_RRC_PCIRange {
		ptrLTENR_RRC_PhysCellID start;
		LTENR_RRC_Range range;
		_ptr_ele ele;//list
	}LTENR_RRC_PCIRange, * ptrLTENR_RRC_PCIRange;

	typedef struct stru_LTENR_RRC_IntraFreqBalckCell {
		ptrLTENR_RRC_PCIRange PCIRange;//list
		_ptr_ele ele;//list
	}LTENR_RRC_IntraFreqBalckCell, * ptrLTENR_RRC_IntraFreqBalckCell;

	typedef struct stru_LTENR_RRC_IntraFreqNeighCell {
		ptrLTENR_RRC_PhysCellID physCellId;
		_ptr_ele ele;//list
	}LTENR_RRC_IntraFreqNeighCell, * ptrLTENR_RRC_IntraFreqNeighCell;

	typedef struct stru_LTENR_RRC_SIB3MSG {
		ptrLTENR_RRC_IntraFreqNeighCell intraFreqNeighCell;//list
		ptrLTENR_RRC_IntraFreqBalckCell intraFreqBlackCell;//list
	}LTENR_RRC_SIB3MSG, * ptrLTENR_RRC_SIB3MSG;

#pragma endregion

#pragma region SIB4_MSG_Structure
	typedef struct stru_LTENR_RRC_ARFCNValueNR {
		UINT arfanValueNr : 4;
	}LTENR_RRC_ARFCNValueNR, * ptrLTENR_RRC_ARFCNValueNR;

	typedef struct stru_LTENR_RRC_InterNeighCellInfo {
		ptrLTENR_RRC_PhysCellID physCellId;
		LTENR_RRC_QOffsetRange qOffsetCell;
		UINT qRxLevMinOffsetCell : 4;
		UINT qRxLevMinOffsetCellSUL : 4;
		UINT qQualMinOffsetCell : 4;
		_ptr_ele ele;//list
	}LTENR_RRC_InterNeighCellInfo, * ptrLTENR_RRC_InterNeighCellInfo;

	typedef struct stru_LTENR_RRC_InterFreqBalckCell {
		ptrLTENR_RRC_PCIRange PCIRange;//list
		_ptr_ele ele;//list
	}LTENR_RRC_InterFreqBalckCell, * ptrLTENR_RRC_InterFreqBalckCell;

	typedef struct stru_LTENR_RRC_InterFreqCarrierFreqInfo {
		ptrLTENR_RRC_ARFCNValueNR dlCarrierFreq;
		ptrTENR_RRC_NRMultiBandInfo freqBand;//list
		ptrTENR_RRC_NRMultiBandInfo freqBandSUL;//list
		UINT noofSSBlockToAverage : 4;
		ptrLTENR_RRC_ThresholdNR absThreshSSBlockConsolidation;
		ptrLTENR_RRC_SSBMTC smtc;
		LTENR_RRC_SubCarrierSpacing ssbSubCarrierSpacing;
		ptrLTENR_RRC_SSBToMeasure ssbToMeasure;
		bool deriveSSBIndexFromCell;
		ptrLTENR_RRC_SSRSSIMeasurement ssRSSIMeasurement;
		ptrLTENR_RRC_QRxLevMin qRxLevMin;
		ptrLTENR_RRC_QRxLevMin qRxLevMinSUL;
		int qQualMin;
		ptrLTENR_RRC_PMax pMax;
		ptrLTENR_RRC_TReselection tReselectionNR;
		ptrLTENR_RRC_SpeedStateScaleFactors tReselectionNRSF;
		ptrLTENR_RRC_ReselectionThreshold threshXHighP;
		ptrLTENR_RRC_ReselectionThreshold threshXLowP;
		ptrLTENR_RRC_ReselectionThresholdQ threshXQ;
		ptrLTENR_RRC_CellReselectionPriority cellReselectionPriority;
		LTENR_RRC_CellReselectionSubPriority cellReselectionSubPriority;
		LTENR_RRC_QOffsetRange qOffsetFreq;
		ptrLTENR_RRC_InterNeighCellInfo interFreqNeighCell;//list
		ptrLTENR_RRC_InterFreqBalckCell interFreqBlackCell;//list

		_ptr_ele ele;//list
	}LTENR_RRC_InterFreqCarrierFreqInfo, * ptrLTENR_RRC_InterFreqCarrierFreqInfo;

	typedef struct stru_LTENR_RRC_SIB4MSG {
		ptrLTENR_RRC_InterFreqCarrierFreqInfo interFreqCarrierFreq;//list
		char lateNonCriticalExtension[8];
	}LTENR_RRC_SIB4MSG, * ptrLTENR_RRC_SIB4MSG;

#pragma endregion

#pragma region System_Information_MSG_Structure
	typedef struct stru_LTENR_RRC_SystemInformation {
		ptrLTENR_RRC_SIB2MSG sib2;
		ptrLTENR_RRC_SIB3MSG sib3;
		ptrLTENR_RRC_SIB4MSG sib4;
	}LTENR_RRC_SIMSG, * ptrLTENR_RRC_SIMSG;
#pragma endregion

#pragma region RRC_SETUP_REQUEST

	typedef enum enum_LTENR_RRC_EstablishmentCause {
		EstablishmentCause_Emergency,
		EstablishmentCause_HighPriorityAccess,
		EstablishmentCause_mt_Access,
		EstablishmentCause_mo_Signalling,
		EstablishmentCause_mo_data,
		EstablishmentCause_mo_Videocall,
		EstablishmentCasue_mo_VoiceCall,
		Establishmentcause_mo_SMS,
		EstablishmentCause_mps_PriorityAccess,
		EstablishmentCause_mcs_PriorityAccess,
		spare6, spare5, spare4, spare3, spare2, spare1
	}LTENR_RRC_EstablishmentCause;
	static const char* strLTENR_RRC_EstablishmentCause[] = {
		"Emergency","HighPriorityAccess","mt_Access","mo_Signalling","mo_data",
		"mo_Videocall","mo_VoiceCall","mo_SMS","mps_PriorityAccess",
		"mcs_PriorityAccess"
	};


	typedef struct stru_LTENR_RRC_SETUP_REQUEST {
		NETSIM_ID ueIdentity;
		LTENR_RRC_EstablishmentCause establishmentCause;
	}LTENR_RRC_SETUP_REQUEST, * ptrLTENR_RRC_SETUP_REQUEST;

#pragma endregion

#pragma region PDCP_Config

	typedef enum enum_LTENR_RRC_DiscardTimer {
		DiscardTimer_ms10,
		DiscardTimer_ms20,
		DiscardTimer_ms30,
		DiscardTimer_ms40,
		DiscardTimer_ms50,
		DiscardTimer_ms60,
		DiscardTimer_ms75,
		DiscardTimer_ms100,
		DiscardTimer_ms150,
		DiscardTimer_ms200,
		DiscardTimer_ms250,
		DiscardTimer_ms300,
		DiscardTimer_ms500,
		DiscardTimer_ms750,
		DiscardTimer_ms1500,
		DiscardTimer_infinity,
	}LTENR_RRC_DiscardTimer;

	typedef enum enum_LTENR_RRC_PDCPSNSizeUL {
		PDCPSNSizeUL_len12bits,
		PDCPSNSizeUL_len18bits,
	}LTENR_RRC_PDCPSNSizeUL;

	typedef enum enum_LTENR_RRC_PDCPSNSizeDL {
		PDCPSNSizeDL_len12bits,
		PDCPSNSizeDL_len18bits,
	}LTENR_RRC_PDCPSNSizeDL;

	typedef struct stru_LTENR_RRC_UplinkOnlyROHCProfiles {
		bool profileOxOOO6;
	}LTENR_RRC_UplinkOnlyROHCProfiles, * ptrLTENR_RRC_UplinkOnlyROHCProfiles;

	typedef struct stru_LTENR_RRC_UpLinkOnlyROHC {
		UINT maxCID : 4;
		ptrLTENR_RRC_UplinkOnlyROHCProfiles profiles;
		bool  drbContinueROHC;
	}LTENR_RRC_UpLinkOnlyROHC, * ptrLTENR_RRC_UpLinkOnlyROHC;

	typedef enum enum_LTENR_RRC_IntegrityProtection {
		IntegrityProtection_enabled,
	}LTENR_RRC_IntegrityProtection;

	typedef struct stru_LTENR_RRC_DRB {
		LTENR_RRC_DiscardTimer discartTimer;
		LTENR_RRC_PDCPSNSizeUL pdcpSNSizeUL;
		LTENR_RRC_PDCPSNSizeDL pdcpSNSizeDL;
		LTENR_RRC_IntegrityProtection integrityProtection;
		bool statusReportRequired;
		bool outOfOrderDelivery;
	}LTENR_RRC_DRB, * ptrLTENR_RRC_DRB;

	typedef struct stru_LTENR_RRC_PDCPConfig {
		ptrLTENR_RRC_DRB drb;
	}LTENR_RRC_PDCPConfig, * ptrLTENR_RRC_PDCPConfig;
#pragma endregion

#pragma region RRCSetup

	typedef struct stru_LTENR_RRC_RRCTransactionIdentifier {
		UINT RRCTransactionIdentifier : 4;
		/*
		0 : Any other msg except following one
		1 : RRC Reestablishment, RRC Resume, RRC Reconfig,
		2 : UE Capability, DL Info Transfer,
		3 : RRC Setup, RRC setup complete
		*/
	}LTENR_RRC_RRCTransactionIdentifier, * ptrLTENR_RRC_RRCTransactionIdentifier;

	typedef struct stru_LTENR_RRC_SRBIdentity {
		UINT SRBIdentity : 4;
	}LTENR_RRC_SRBIdentity, * ptrLTENR_RRC_SRBIdentity;

	typedef struct stru_LTENR_RRC_SRBToAddMod {
		ptrLTENR_RRC_SRBIdentity srbIdentity;
		bool reesablishmentPDCP;
		bool diacaedONPDCP;
		ptrLTENR_RRC_PDCPConfig pdcpConfig;
		_ptr_ele ele;
	}LTENR_RRC_SRBToAddMod, * ptrLTENR_RRC_SRBToAddMod;

	typedef struct stru_LTENR_RRC_PDUSessionID {
		UINT pduSession : 4;
	}LTENR_RRC_PDUSessionID, * ptrLTENR_RRC_PDUSessionID;

	typedef struct stru_LTENR_RRC_QFI {
		UINT qfi : 4;
	}LTENR_RRC_QFI, * ptrLTENR_RRC_QFI;

	typedef enum enum_LTENR_RRC_SADPHeaderDL {
		SADPHeaderDL_present, SADPHeaderDL_absent,
	}LTENR_RRC_SADPHeaderDL;

	typedef enum enum_LTENR_RRC_SADPHeaderUL {
		SADPHeaderUL_present, SADPHeaderUL_absent,
	}LTENR_RRC_SADPHeaderUL;

	typedef struct stru_LTENR_RRC_SADPConfig {
		ptrLTENR_RRC_PDUSessionID pduSession;
		LTENR_RRC_SADPHeaderDL sdapHeaderDL;
		LTENR_RRC_SADPHeaderUL sadpHeaderUL;
		bool defaultDRB;
		ptrLTENR_RRC_QFI mappedQoSFlowsToAdd;
		ptrLTENR_RRC_QFI mappedQoSFlowsToRelease;
	}LTENR_RRC_SADPConfig, * ptrLTENR_RRC_SADPConfig;

	typedef struct stru_LTENR_RRC_DRBIdentity {
		UINT drbIdentity : 4;
	}LTENR_RRC_DRBIdentity, * ptrLTENR_RRC_DRBIdentity;

	typedef struct stru_LTENR_RRC_DRBToAddMod {
		ptrLTENR_RRC_SADPConfig SADPConfig; //choice 
		ptrLTENR_RRC_DRBIdentity drbIdentity;
		bool reestablishmentPDCP;
		bool recoverPDCP;
		ptrLTENR_RRC_PDCPConfig PDCPConfig;
		_ptr_ele ele;
	}LTENR_RRC_DRBToAddMod, * ptrLTENR_RRC_DRBToAddMod;

	typedef struct stru_LTENR_RRC_DRBTORelease {
		ptrLTENR_RRC_DRBIdentity drbIdentity;
		_ptr_ele ele;
	}LTENR_RRC_DRBTORelease, * ptrLTENR_RRC_DRBTORelease;

	typedef struct stru_LTENR_RRC_RadioBearerConfig {
		ptrLTENR_RRC_SRBToAddMod srbToAddMod;
		bool srb3ToRelease;
		ptrLTENR_RRC_DRBToAddMod drbToAddModList;//NOT REQUIRED IN NETSIM 
		ptrLTENR_RRC_DRBTORelease drbToRelease;//NOT REQUIRED IN NETSIM
	}LTENR_RRC_RadioBearerConfig, * ptrLTENR_RRC_RadioBearerConfig;

	typedef struct stru_LTENR_RRC_RRCSetupIE {
		ptrLTENR_RRC_RadioBearerConfig radioBearerConfig;
		UINT masterCellGroup : 8;
		//ignoring critical and non critical extension
	}LTENR_RRC_RRCSetupIE, * ptrLTENR_RRC_RRCSetupIE;

	typedef enum enum_LTENR_RRC_RRCSetup_Response {
		RRCSETUP_RESPONSE_OF_RRCSETUPREQUEST,
		RRCSETUP_RESPONSE_OF_RRCRESUMEREQUEST,
		RRCSETUP_RESPONSE_OF_RRCREESTABLISHMENTREUEST
	}LTENR_RRC_RRCSetup_Response;
	static const char* strLTENR_RRC_RRCSetup_Response[] = {
		"RRCSETUPREQUEST","RRCRESUMEREQUEST","RRCREESTABLISHMENTREUEST"
	};
	typedef struct stru_LTENR_RRC_RRCSetup {
		ptrLTENR_RRC_RRCTransactionIdentifier rrcTransactionIdentifier;
		ptrLTENR_RRC_RRCSetupIE rrcSetup;
		LTENR_RRC_RRCSetup_Response rrcResponsetype;
		ptrLTENR_PDCPVAR PDCPVAR;
		ptrLTENR_PDCPENTITY pdcpEntity;
		ptrLTENR_RLCVAR RLCVAR;
	}LTENR_RRC_RRCSetup, * ptrLTENR_RRC_RRCSetup;
#pragma endregion

#pragma region RRCSetupComplete 

	typedef struct stru_LTENR_RRC_RegisteredAMF {
		ptrLTENR_RRC_PLMNIdentity plmnIdentity;
		UINT amfIdentity;
	}LTENR_RRC_RegisteredAMF, * ptrLTENR_RRC_RegisteredAMF;

	/*
	sst refers to expected behaviour of the slice
	sst : 1 slice suitable for the handling of 5G enhanced Mobile Broadband(eMBB)
	sst : 2 slice suitable for handling of ultra reliable low latency communication(URLLC)
	sst : 3 slice suitable for the handling of massive IoT(MIoT)
	*/
	typedef struct stru_LTENR_RRC_SNSSAI {
		UINT sst : 8;
		UINT sstSD : 32; // optional
	}LTENR_RRC_SNSSAI, * ptrLTENR_RRC_SNSSAI;

	typedef enum enum_LTENR_RRC_GaumiType {
		GaumiType_native,
		GaumiType_mapped,
	}LTENR_RRC_GaumiType;
	static const char* strLTENR_RRC_GaumiType[] = {
		"Native","Mapped"
	};
	typedef struct stru_LTENR_RRC_NG5GSTMSIValue {
		int ng5GSTMSI;
		UINT ng5GSTMSIPart2 : 9;
	}LTENR_RRC_NG5GSTMSIValue, * ptrLTENR_RRC_NG5GSTMSIValue;

	typedef struct stru_LTENR_RRC_RRCSetupCompleteIEs {
		UINT selectedPLMNIdentity;
		UINT amfIdentifier;
		LTENR_RRC_GaumiType gaumiType;
		UINT dedicatedNASMessage : 8;
	}LTENR_RRC_RRCSetupCompleteIEs, * ptrLTENR_RRC_RRCSetupCompleteIEs;

	typedef struct stru_LTENR_RRC_RRCSetupComplete {
		ptrLTENR_RRC_RRCTransactionIdentifier rrcTransactionIdentifier;
		ptrLTENR_RRC_RRCSetupCompleteIEs rrcSetupCompleteIEs;
	}LTENR_RRC_RRCSetupComplete, * ptrLTENR_RRC_RRCSetupComplete;

#pragma endregion

#pragma region RRCReject

	typedef struct stru_LTENR_RRC_RejectWaitTime {
		UINT rejectWaitTime : 4;
	}LTENR_RRC_RejectWaitTime, * ptrLTENR_RRC_RejectWaitTime;

	typedef struct stru_LTENR_RRC_RRCReject {
		ptrLTENR_RRC_RejectWaitTime waitTime;
	}LTENR_RRC_RRCReject, * ptrLTENR_RRC_RRCReject;

#pragma endregion

#pragma region RRC_ReConfiguration

	typedef struct stru_LTENR_RRC_PCIRangeIndex {
		UINT PCIRangeIndex : 4;
		_ptr_ele ele;
	}LTENR_RRC_PCIRangeIndex, * ptrLTENR_RRC_PCIRangeIndex;

	typedef struct stru_LTENR_RRC_PCIRangeElement {
		ptrLTENR_RRC_PCIRangeIndex pciRangeIndex;
		ptrLTENR_RRC_PCIRange pciRange;
		_ptr_ele ele;
	}LTENR_RRC_PCIRangeElement, * ptrLTENR_RRC_PCIRangeElement;

	typedef struct stru_LTENR_RRC_CellsToAddMod {
		ptrLTENR_RRC_PhysCellID physCellId;
		LTENR_RRC_QOffsetRange cellIndividualOffset;
		_ptr_ele ele;
	}LTENR_RRC_CellsToAddMod, * ptrLTENR_RRC_CellsToAddMod;

	typedef struct stru_LTENR_RRC_MeasObjectNR {
		ptrLTENR_RRC_ARFCNValueNR ssbFrequency;
		LTENR_RRC_SubCarrierSpacing ssbSubCarrierSpacing;
		ptrLTENR_RRC_SSBMTC smtc1;
		ptrLTENR_RRC_SSBMTC smtc2;
		ptrLTENR_RRC_ARFCNValueNR refFreqCSIRS;
		ptrLTENR_RRC_ThresholdNR absThreshSSBlocksConsolidation;
		ptrLTENR_RRC_ThresholdNR absThreshCSIRSConsolidation;
		UINT nrofSSBlockToAverage : 4;
		UINT nrofCSIRSResourceToAverage : 4;
		UINT quantityConfigIndex : 4;
		LTENR_RRC_QOffsetRange offsetMO;
		ptrLTENR_RRC_PCIRange cellsToRemove;
		ptrLTENR_RRC_CellsToAddMod cellsToAddModList;
		ptrLTENR_RRC_PCIRangeIndex blackCellsToRemove;
		ptrLTENR_RRC_PCIRangeElement blackCellsToAddMod;
		ptrLTENR_RRC_PCIRangeIndex whiteCellsToRemove;
		ptrLTENR_RRC_PCIRangeElement whiteCellsToAddMod;
		//freqbandindicator and meascyclescell not added
	}LTENR_RRC_MeasObjectNR, * ptrLTENR_RRC_MeasObjectNR;

	typedef struct stru_LTENR_RRC_MeasObjectID {
		UINT measObjectID : 4;
	}LTENR_RRC_MeasObjectID, * ptrLTENR_RRC_MeasObjectID;

	typedef struct stru_LTENR_RRC_MeasObjectToAdded {
		ptrLTENR_RRC_MeasObjectID measObjectIDNode;
		ptrLTENR_RRC_MeasObjectNR measObjectNR;
	}LTENR_RRC_MeasObjectToAdded, * ptrLTENR_RRC_MeasObjectToAdded;

	typedef struct stru_LTENR_RRC_MeasObjectToAddMod {
		ptrLTENR_RRC_MeasObjectToAdded measObjectToAdded;
		_ptr_ele ele;
	}LTENR_RRC_MeasObjectToAddMod, * ptrLTENR_RRC_MeasObjectToAddMod;

	typedef struct stru_LTENR_RRCMeasObjectToRemove {
		ptrLTENR_RRC_MeasObjectID measObjectId;
		_ptr_ele ele;
	}LTENR_RRCMeasObjectToRemove, * ptrLTENR_RRCMeasObjectToRemove;

	typedef struct stru_LTENR_RRC_MeasConfig {
		ptrLTENR_RRCMeasObjectToRemove measObjectToRemove;
		ptrLTENR_RRC_MeasObjectToAddMod measObjectToAddMod;
	}_LTENR_RRC_MeasConfig, * ptr_LTENR_RRC_MeasConfig;

	typedef struct stru_LTENR_RRC_NextHopChainingCount {
		UINT nextHopChainingCount;
	}LTENR_RRC_NextHopChainingCount, * ptrLTENR_RRC_NextHopChainingCount;

	typedef struct stru_LTENR_RRC_MasterKeyUpdate {
		bool keySetChangeIndicator;
		ptrLTENR_RRC_NextHopChainingCount nextHopChainigCountNode;
	}LTENR_RRC_MasterKeyUpdate, * ptrLTENR_RRC_MasterKeyUpdate;

	typedef struct stru_LTENR_RRC_RRCReconfigurationIEsV1530 {
		char masterCellFroup[8];
		bool fullConfig;
		UINT dedicatedNASMessage : 8;
		ptrLTENR_RRC_MasterKeyUpdate masterKeyUpdate;
		char dedicatedSIB1Delivery[8];
		char dedicatedSystemInformationDelivery[8];
		//1540 critical extension not implemeted
	}LTENR_RRC_RRCReconfigurationIEsV1530, * ptrLTENR_RRC_RRCReconfigurationIEsV1530;

	typedef struct stru_LTENR_RRC_RRCReconfigurationIEs {
		ptrLTENR_RRC_RadioBearerConfig radioBearerConfig;
		char secondaryCellGroup[8];//Containing CellGroupConfig
		ptr_LTENR_RRC_MeasConfig measConfig;
		ptrLTENR_RRC_RRCReconfigurationIEsV1530 nonCriticalExtension;
	}LTENR_RRC_RRCReconfigurationIEs, * ptrLTENR_RRC_RRCReconfigurationIEs;

	typedef struct stru_LTENR_RRC_RRCReconfiguration {
		ptrLTENR_RRC_RRCTransactionIdentifier rrcTranactionIdentifier;
		//ptrLTENR_RRC_RRCReconfigurationIEs rrcReconfigurationIEs; // optional
	}LTENR_RRC_RRCReconfiguration, * ptrLTENR_RRC_RRCReconfiguration;
#pragma endregion

#pragma region RRC_Reconfiguration_Complete

	typedef struct stru_LTENR_RRC_ServCellIndex {
		UINT servCellIndex : 4;
	}LTENR_RRC_ServCellIndex, * ptrLTENR_RRC_ServCellIndex;

	typedef struct stru_LTENR_RRC_BwpId {
		UINT BwpId : 4;
	}LTENR_RRC_BwpId, * ptrLTENR_RRC_BwpId;

	typedef struct stru_LTENR_RRC_UpLinkTxDirectCurrentBWP {
		ptrLTENR_RRC_BwpId bwpId;
		bool shift7dot5kHz;
		UINT txDirectCurrentLocation : 4;
	}LTENR_RRC_UpLinkTxDirectCurrentBWP, * ptrLTENR_RRC_UpLinkTxDirectCurrentBWP;

	typedef struct stru_LTENR_RRC_UpLinkTxDirectCurrentCell {
		ptrLTENR_RRC_ServCellIndex servCellId;
		ptrLTENR_RRC_UpLinkTxDirectCurrentBWP upLinkTxDirectCurrentBWP;
		_ptr_ele ele;
	}LTENR_RRC_UpLinkTxDirectCurrentCell, * ptrLTENR_RRC_UpLinkTxDirectCurrentCell;

	typedef struct stru_LTENR_RRC_RRCReconfigurationCompletev1530IEs {
		ptrLTENR_RRC_UpLinkTxDirectCurrentCell uplinkTxDirectCurrentcell;
	}LTENR_RRC_RRCReconfigurationCompletev1530IEs, * ptrLTENR_RRC_RRCReconfigurationCompletev1530IEs;

	typedef struct stru_LTENR_RRC_RRCReconfigurationCompleteIEs {
		char lateNonCriticalExtension[8];
		ptrLTENR_RRC_RRCReconfigurationCompletev1530IEs nonCriticalExtension;
	}LTENR_RRC_RRCReconfigurationCompleteIEs, * ptrLTENR_RRC_RRCReconfigurationCompleteIEs;

	typedef struct stru_LTENR_RRC_RRCReconfigurationComplete {
		ptrLTENR_RRC_RRCTransactionIdentifier rrcTransactionIdentifier;
		ptrLTENR_RRC_RRCReconfigurationCompleteIEs rrcReconfigurationComplete;
	}LTENR_RRC_RRCReconfigurationComplete, * ptrLTENR_RRC_RRCReconfigurationComplete;

#pragma endregion

#pragma region RRC_Reestablishment

	typedef struct struc_LTENR_RRC_RRCReestablishmentIEs {
		ptrLTENR_RRC_NextHopChainingCount nextHopChainingCount;/*Range can be from 0 to 7*/
	}LTENR_RRC_RRCReestablishmentIEs, * ptrLTENR_RRC_RRCReestablishmentIEs;

	typedef struct stru_LTENR_RRC_RRCReestablishment {
		ptrLTENR_RRC_RRCTransactionIdentifier rrcTransactionIdentifier;
		int nextHopChainingCount;
		//ptrLTENR_RRC_RRCReestablishmentIEs rrcReestablishmentIEs;
	}LTENR_RRC_RRCReestablishment, * ptrLTENR_RRC_RRCReestablishment;

#pragma endregion

#pragma region RRC_Reestablishment_Request

	typedef struct stru_LTENR_RRC_RNTIValue {
		UINT RNTIValue : 4;
	}LTENR_RRC_RNTIValue, * ptrLTENR_RRC_RNTIValue;

	typedef struct stru_LTENR_RRC_ShortMACI {
		char shortMACI[16];
	}LTENR_RRC_ShortMACI, * ptrLTENR_RRC_ShortMACI;

	typedef struct stru_LTENR_RRC_ReestabUEIdentity {
		ptrLTENR_RRC_RNTIValue cRNTI;
		ptrLTENR_RRC_PhysCellID physCellId;
		ptrLTENR_RRC_ShortMACI shortMACI;
	}LTENR_RRC_ReestabUEIdentity, * ptrLTENR_RRC_ReestabUEIdentity;

	typedef enum enum_LTENR_RRC_ReestablishmentCause {
		ReestablishmentCause_reconfigurationFailure,
		ReestablishmentCause_handoverFailure,
		ReestablishmentCause_otherFailure,
		ReestablishmentCause_spare1,
	}LTENR_RRC_ReestablishmentCause;
	static const char* strLTENR_RRC_ReEstablishmentCause[] = {
		"ReconfigurationFailure","handoverFailure","otherFailure","spare1"
	};

	typedef struct stru_LTENR_RRC_RRCReestablishmentRequest {
		NETSIM_ID ueIdentity;
		LTENR_RRC_ReestablishmentCause reRestablishmentCause;
	}LTENR_RRC_RRCReestablishmentRequest, * ptrLTENR_RRC_RRCReestablishmentRequest;

#pragma endregion

#pragma region RRC_ReestablishmentComplete

	typedef struct stru_LTENR_RRC_ReestablishmentComplete {
		ptrLTENR_RRC_RRCTransactionIdentifier rrcTransactionIdentifier;
		//extension not implemeted
	}LTENR_RRC_ReestablishmentComplete, * ptrLTENR_RRC_ReestablishmentComplete;

#pragma endregion

#pragma region RRC_Release

	typedef struct stru_LTENR_RRC_CarrierInfoNR {
		ptrLTENR_RRC_ARFCNValueNR carrierFreq;
		LTENR_RRC_SubCarrierSpacing ssbSubcarrierSpacing;
		ptrLTENR_RRC_SSBMTC smtc;
	}LTENR_RRC_CarrierInfoNR, * ptrLTENR_RRC_CarrierInfoNR;

	typedef struct stru_LTENR_RRC_RedirectedCarrierInfo {
		ptrLTENR_RRC_CarrierInfoNR nr;
		//only nr is implemented EUTRA not implemnted yet!
	}LTENR_RRC_RedirectedCarrierInfo, * ptrLTENR_RRC_RedirectedCarrierInfo;

	typedef struct stru_LTENR_RRC_FreqPriorityNR {
		ptrLTENR_RRC_ARFCNValueNR carrierFreq;
		ptrLTENR_RRC_CellReselectionPriority cellReselectionPriority;
		LTENR_RRC_CellReselectionSubPriority cellResectionSubPriority;
		_ptr_ele ele;
	}LTENR_RRC_FreqPriorityNR, * ptrLTENR_RRC_FreqPriorityNR;

	typedef struct stru_LTENR_RRC_CellReselectionPriorities {
		ptrLTENR_RRC_FreqPriorityNR freqPriorityNR;
	}LTENR_RRC_CellReselectionPriorities, * ptrLTENR_RRC_CellReselectionPriorities;

	typedef struct stru_LTENR_RRC_ShortIRNTI {
		char shortIRNTI[24];
	}LTENR_RRC_ShortIRNTI, * ptrLTENR_RRC_ShortIRNTI;

	typedef enum enum_LTENR_RRC_PeriodicRNAUimerValue {
		PeriodicRNAUimerValue_min5,
		PeriodicRNAUimerValue_min10,
		PeriodicRNAUimerValue_min20,
		PeriodicRNAUimerValue_min30,
		PeriodicRNAUimerValue_min60,
		PeriodicRNAUimerValue_min120,
		PeriodicRNAUimerValue_min360,
		PeriodicRNAUimerValue_min720,
	}LTENR_RRC_PeriodicRNAUimerValue;

	typedef struct stru_LTENR_RRC_SuspendConfig {
		int fullIRNTI;
		ptrLTENR_RRC_ShortIRNTI shortIRNTIValue;
		LTENR_RRC_pagingCycle ranPagingCycle;
		//Tracking area not implemented so that RANNotificationAreaInfo variable not added
		LTENR_RRC_PeriodicRNAUimerValue t380;
		ptrLTENR_RRC_NextHopChainingCount nextHopChainingCount;
	}LTENR_RRC_SuspendConfig, * ptrLTENR_RRC_SuspendConfig;

	typedef enum enum_LTENR_RRC_DeprioritisationType {
		deprioritisationType_frequency,
		deprioritisationType_nr,
	}LTENR_RRC_DeprioritisationType;

	typedef enum enum_LTENR_RRC_DeprioritisationTimer {
		deprioritisationTimer_min5,
		deprioritisationTimer_min10,
		deprioritisationTimer_min15,
		deprioritisationTimer_min30,
	}LTENR_RRC_DeprioritisationTimer;

	typedef struct stru_LTENR_RRC_DeprioritisationReq {
		LTENR_RRC_DeprioritisationType deprioritisationType;
		LTENR_RRC_DeprioritisationTimer deprioritisationTimer;
	}LTENR_RRC_DeprioritisationReq, * ptrLTENR_RRC_DeprioritisationReq;

	typedef struct stru_LTENR_RRC_RRCRejectReleasev1504IEs {
		ptrLTENR_RRC_RejectWaitTime rejectWaitTime;
	}LTENR_RRC_RRCRejectReleasev1504IEs, * ptrLTENR_RRC_RRCRejectReleasev1504IEs;

	typedef struct stru_LTENR_RRC_RRCReleaseIEs {
		ptrLTENR_RRC_RedirectedCarrierInfo redirectedCarrierInfo;
		ptrLTENR_RRC_CellReselectionPriorities cellReselectionPriorities;
		ptrLTENR_RRC_SuspendConfig suspendConfig;
		ptrLTENR_RRC_DeprioritisationReq deprioritisationReq;
		ptrLTENR_RRC_RRCRejectReleasev1504IEs rrcRejectReleasev1504IEs;
	}LTENR_RRC_RRCReleaseIEs, * ptrLTENR_RRC_RRCReleaseIEs;

	typedef struct stru_LTENR_RRC_RRCRelease {
		ptrLTENR_RRC_RRCTransactionIdentifier rrcTransactionIdentifier;
		ptrLTENR_RRC_RRCReleaseIEs rrcReleaseIEs;
	}LTENR_RRC_RRCRelease, * ptrLTENR_RRC_RRCRelease;

#pragma endregion

#pragma region UE_MEASUREMENT_REPORT

	typedef struct stru_LTENR_RRC_MeasQuantityResults {
		double rsrp;/*range is from 0 to 127*/
		double rsrq;
		double sinr;
	}LTENR_RRC_MeasQuantityResults, * ptrLTENR_RRC_MeasQuantityResults;

	typedef enum enum_LTENR_RRC_NR_RS_TYPE {
		RS_TYPE_SSB,
		RS_TYPE_CSI_RS,
		RS_TYPE_SPARE1,
	}LTENR_RRC_NR_RS_TYPE;
	static const char* strLTENR_RRC_NR_RS_TYPE[] = {
		"SSB","SCI_RS","spare1"
	};

	typedef enum enum_LTENR_RRC_ReportInterval {
		ReportInterval_ms120, ReportInterval_ms240, ReportInterval_ms480,
		ReportInterval_ms640, ReportInterval_ms1024, ReportInterval_ms2048,
		ReportInterval_ms5120, ReportInterval_ms10240, ReportInterval_ms20480,
		ReportInterval_ms40960, ReportInterval_min1, ReportInterval_min6,
		ReportInterval_min12, ReportInterval_min30,
	}LTENR_RRC_ReportInterval;
	static const char* strLTENR_RRC_ReportInterval[] = {
		"ms120", "ms240", "ms480", "ms640", "ms1024", "ms2048", "ms5120", "ms10240", "ms20480",
		"ms40960", "min1", "min6", "min12", "min30"
	};

	typedef enum enum_LTENR_RRC_ReportAmount {
		ReportAmount_r1,
		ReportAmount_r2,
		ReportAmount_r4,
		ReportAmount_r8,
		ReportAmount_r16,
		ReportAmount_r32,
		ReportAmount_r64,
		ReportAmount_infinity
	}LTENR_RRC_ReportAmount;
	static const char* strLTENR_RRC_ReportAmount[] = {
		"r1", "r2", "r4", "r8", "r16", "r32", "r64", "INF"
	};

	typedef struct stru_LTENR_RRC_UE_MEASUREMENT_REPORT {
		LTENR_RRC_NR_RS_TYPE rs_type;
		double reportInteval;
		LTENR_RRC_ReportAmount reportAmount;
		NETSIM_ID cellID;
		NETSIM_ID cellIF;
		NETSIM_ID ueID;
		double sinr;
		double rsrp;
		double rsrq;
		/*
		ptrLTENR_RRC_MeasQuantityResults  resultsSSBCell;
		ptrLTENR_RRC_MeasQuantityResults  resultsCSIRSCell;
		*/
		_ele* ele;
	}LTENR_RRC_UE_MEASUREMENT_REPORT, * ptrLTENR_RRC_UE_MEASUREMENT_REPORT;

#define MEASUREMENT_REPORT_ALLOC() (struct stru_LTENR_RRC_UE_MEASUREMENT_REPORT*)list_alloc(sizeof(struct stru_LTENR_RRC_UE_MEASUREMENT_REPORT),offsetof(struct stru_LTENR_RRC_UE_MEASUREMENT_REPORT,ele))
#define MEASUREMENT_REPORT_REMOVE(ls, mem) (LIST_FREE((void**)(ls),(mem)))
#pragma endregion

#pragma region RRC_Entity

	typedef struct stru_LTENR_GNBRRC
	{
		NETSIM_ID d;
		NETSIM_ID in;
		ptrLTENR_RRC_MIBMSG MIB;
		ptrLTENR_RRC_SIB1MSG SIB1;
		ptrLTENR_RRC_SIMSG SI;
		double T300;
		double ueMeasReportInterval;
		double mibInterval;
		double sib1Interval;
	}LTENR_GNBRRC, * ptrLTENR_GNBRRC;
	typedef struct stru_LTENR_SINREPORT {
		double sinr;
		NETSIM_ID cellID;
		_ele* ele;
	}LTENR_SINREPORT, * ptr_LTENR_SINREPORT;
#define SINR_REPORT_ALLOC() (struct stru_LTENR_SINREPORT*)list_alloc(sizeof(struct stru_LTENR_SINREPORT),offsetof(struct stru_LTENR_SINREPORT,ele))
#define SINR_REPORT_REMOVE(ls, mem) (LIST_FREE((void**)(ls),(mem)))
	typedef struct stru_LTENR_UERRC
	{
		NETSIM_ID d;
		NETSIM_ID in;
		ptrLTENR_RRC_MIBMSG MIB;
		ptrLTENR_RRC_SIB1MSG SIB1;
		ptrLTENR_RRC_SIMSG SI;
		ptrLTENR_RRC_SETUP_REQUEST rrcSetupRequest;
		LTENR_UE_RRCSTATE ueRRCState;
		NETSIM_ID SelectedCellID;
		NETSIM_ID SelectedCellIF;
		NETSIM_ID pCell;
		ptr_LTENR_SINREPORT SINRReport;
		double T300TimerValue;
		bool isT300Running;
		UINT64 T300ID;

		double T301TimerValue;
		bool isT301Running;
		UINT64 T301ID;

		double T311TimerValue;
		bool isT311Running;
		UINT64 T311ID;

		UINT64 ueMeasurementID;
	}LTENR_UERRC, * ptrLTENR_UERRC;

	typedef struct stru_LTENR_RRC_Hdr {

		LTENR_MSGTYPE msgType;
		void* msg;
		NETSIM_ID SenderID;
		NETSIM_ID SenderIF;
	}LTENR_RRC_Hdr, * ptrLTENR_RRC_Hdr;
#pragma endregion

#pragma region Variable_Def_For_RRC
	UINT mibHdrID;
	UINT sib1HdrID;
	UINT siHdrID;
	UINT rrcSetupRequestID;
	UINT rrcSetupID;
	UINT rrcSetupCompleteID;
	UINT ueMEASID;
	UINT rrcReestablishmentRequestID;
	UINT rrcReestablishmentID;
	UINT rrcReconfigurationID;
	UINT nasHandoverRequest;
	UINT nasHandoverRequestAck;
	UINT nasHandoverTrigger;
	UINT nasPathSwitch;
	UINT naspathSwitchAck;
	UINT nasUEContextRelease;
	UINT nasUEContextReleaseACK;
	UINT nasHandoverComplete;

	void fn_NetSim_LTENR_GNBRRC_GenerateMIB();
	void fn_NetSim_LTENR_GNBRRC_GenerateSIB1();
	void fn_NETSIM_LTENR_SUBEVENT_GENERATE_SI();
	void fn_NetSim_LTENR_RRC_GENERATE_RRC_CONN_REQUEST();
	void fn_NetSIM_LTENR_RRC_GENERATE_RRCSETUP();
	void fn_NetSim_LTENR_RRC_GENERATE_RRC_SETUP_COMPLETE();
	void fn_NetSim_LTENR_RRC_UE_Measurement_Report_Request_Handle();
	void fn_NetSIM_LTENR_RRC_GENERATE_UE_MEASUREMENT_REPORT();
	void  fn_NetSIM_LTENR_RRC_GENERATE_RECONFIGURATION();
	//void fn_NetSIM_LTENR_RRC_GENERATE_REESTABLISHMENT_REQUEST();
	void  fn_NetSIM_LTENR_RRC_GENERATE__REESTABLISHMENT();
	void fn_NetSim_LTENR_RRC_UE_MEASUREMENT_REPORT_REQUEST(NETSIM_ID gnbId, NETSIM_ID gnbInterface);
	void fn_NetSim_LTENR_UERRC_MIB_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_MIBMSG MIB);
	void fn_NetSim_LTENR_UERRC_SIB1_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_SIB1MSG SIB1);
	void fn_NetSim_LTENR_RRC_SI_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_SIMSG SI);
	void fn_NetSim_LTENR_RRC_SETUP_REQUEST_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_SETUP_REQUEST setupReq);
	void fn_NetSim_LTENR_RRC_SETUP_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_RRCSetup rrcSetup);
	void fn_NetSim_LTENR_RRC_RRCSETUPCOMPLETE_RECV(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID ueID, NETSIM_ID ueIF);
	void fn_NetSim_LTENR_RRC_UE_MEASUREMENT_RECV(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID ueID, NETSIM_ID ueIF, ptrLTENR_RRC_UE_MEASUREMENT_REPORT msg);
	void fn_NetSim_LTENR_RRC_UE_Measurement_Report(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID UEID, NETSIM_ID ueIF);
	//void fn_NetSim_LTENR_RRC_REESTABLISHMENT_REQUEST(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF);
	void fn_NetSim_LTENR_RRC_REESTABLISHMENT_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_RRCReestablishment msg);
	void fn_NetSim_LTENR_RRC_RECONFIGURATION_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_RRCReconfiguration msg);
	void LTENR_RRCMSG_INIT();

	void LTENR_RRC_MIB_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_RRC_SIB1_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_RRC_SI_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_RRC_SETUP_REQUEST_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_RRC_UE_MEASUREMENT_REPORT_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_RRC_SETUP_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_RRRC_SETUP_COMPLETE_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_NAS_HANDOVER_REQUEST_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_NAS_HANDOVER_REUQEST_ACK_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_NAS_HANDOVER_COMMAND_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_RRRC_RECONFIGURATION_REQUEST_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_NAS_PATH_SWITCH_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_NAS_PATH_SWITCH_ACK_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_NAS_UE_CONTEXT_RELEASE_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);
	void LTENR_NAS_UE_CONTEXT_RELEASE_ACK_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);

	void LTENR_NASUE_CONTEXT_RELEASE_ACK_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_NASUE_CONTEXT_RELEASE_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_NAS_PATH_SWTICH_ACK_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_NAS_PATH_SWITCH_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_NAS_HANDOVER_COMMAND_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_NAS_HANDOVER_REUQEST_ACK_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_NAS_HANDOVER_REQUEST_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_RECONFIGURATION_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_RECONFIGURATION_REQUEST_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_REESTABLISHMENT_REQUEST_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_SETUP_COMPLETE_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_UE_MEASUREMENT_REPORT_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_SETUP_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_SETUP_REQUEST_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_SI_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_SIB1_FREEHDR(ptrLTENR_RRC_Hdr hdr);
	void LTENR_RRC_MIB_FREEHDR(ptrLTENR_RRC_Hdr hdr);

	ptrLTENR_RRC_Hdr LTENR_RRC_MIB_COPY(ptrLTENR_RRC_Hdr hdr);
	ptrLTENR_RRC_Hdr LTENR_RRC_SIB1_COPY(ptrLTENR_RRC_Hdr hdr);
	ptrLTENR_RRC_Hdr LTENR_RRC_SETUP_REQUEST_COPY(ptrLTENR_RRC_Hdr hdr);
	ptrLTENR_RRC_Hdr LTENR_RRC_SETUP_COPY(ptrLTENR_RRC_Hdr hdr);
	ptrLTENR_RRC_Hdr LTENR_RRC_SETUP_COMPLETE_COPY(ptrLTENR_RRC_Hdr hdr);
	ptrLTENR_RRC_Hdr LTENR_RRC_RECONFIGURATION_REQUEST_COPY(ptrLTENR_RRC_Hdr hdr);
	ptrLTENR_RRC_Hdr LTENR_NAS_HANDOVER_REQUEST_COPY(ptrLTENR_RRC_Hdr hdr);
	
	ptrLTENR_RRC_Hdr LTENR_RRC_HDR_GET(NetSim_PACKET* packet, UINT hdrID);
	void LTENR_RRC_HDR_REMOVE(NetSim_PACKET* packet, UINT hdrID);
	void fn_NetSim_LTENR_RRC_TimerEvent_Init();
	void LTENR_RRC_Start_T300(ptrLTENR_UERRC entity);
	void LTENR_RRC_STOP_T300(ptrLTENR_UERRC entity);
	void fn_NetSim_LTENR_RRC_TIMER_T300_HANDLE();
	void LTENR_RRC_Start_T311(ptrLTENR_UERRC entity);
	void LTENR_RRC_STOP_T311(ptrLTENR_UERRC entity);
	void fn_NetSim_LTENR_RRC_TIMER_T311_HANDLE();
	void LTENR_RRRC_REESTABLISHMENT_REQUEST_TRACE(ptrLTENR_RRC_Hdr hdr, char* s);

	void fn_NetSim_LTENR_RRC_RECONFIGURATION(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF);
	void fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(NetSim_PACKET* packet, ptrLTENR_RRC_Hdr hdr, UINT hdrID, LTENR_MSGTYPE type);
#pragma endregion
#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_GNBRRC_H_
