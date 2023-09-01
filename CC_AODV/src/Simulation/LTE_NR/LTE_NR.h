#pragma once
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
#ifndef _NETSIM_LTE_NR_H_
#define _NETSIM_LTE_NR_H_

#include "List.h"

#ifdef  __cplusplus
extern "C" {
#endif

	typedef enum enum_LTE_NR_DEVICE_TYPE
	{
		LTENR_DEVICETYPE_UE = 0,
		LTENR_DEVICETYPE_GNB = 1,
		LTENR_DEVICETYPE_ENB = 1,
		LTENR_DEVICETYPE_EPC = 2,
		LTENR_DEVICETYPE_LAST, //Keep me at last
	}LTENR_DEVICETYPE;
	static const char* strLTENR_DEVICETYPE[] =
	{ "UE","gNB","EPC","Unknown" };

	typedef enum enum_LTENR_LogicalChannel
	{
		LTENR_LOGICALCHANNEL_NONE,
		LTENR_LOGICALCHANNEL_PCCH, // Paging Control Channel
		LTENR_LOGICALCHANNEL_BCCH, // Broadcast Control Channel
		LTENR_LOGICALCHANNEL_CCCH, // Common Control Channel
		LTENR_LOGICALCHANNEL_DCCH, // Dedicated Control Channel 
		LTENR_LOGICALCHANNEL_DTCH, // Dedicated Traffic Channel
		LTENR_LOGICALCHANNEL_LAST, // Keep me at last
	}LTENR_LOGICALCHANNEL;
	static const char* strLTENR_LOGICALCHANNEL[] =
	{ "NONE","PCCH","BCCH","CCCH","DCCH","DTCH","Unknown" };

	typedef enum enum_LTENR_SRB
	{
		LTENR_SRB0,
		LTENR_SRB1,
		LTENR_SRB2,
		LTENR_NA,
		LTENR_LAST,
	}LTENR_SRB;
	static const char* strLTENR_SRB[] =
	{ "SRB0","SRB1","SRB2","N/A","Unknown" };

	typedef enum enum_LTENR_TransportChannel
	{
		LTENR_TRANSPORTCHANNEL_NONE,
		LTENR_TRANSPORTCHANNEL_BCH, // Broadcast Channel
		LTENR_TRANSPORTCHANNEL_PCH, // Paging Channel
		LTENR_TRANSPORTCHANNEL_DL_SCH, // Downlink Shared Channel
		LTENR_TRANSPORTCHANNEL_UL_SCH, // Uplink Shared Channel
		LTENR_TRANSPORTCHANNEL_RACH, // Random Access Channel
		LTENR_TRANSPORTCHANNEL_LAST, // Keep me at last
	}LTENR_TRANSPORTCHANNEL;
	static const char* strLTENR_TRANSPORTCHANNEL[] =
	{ "NONE","BCH","PCH","DL-SCH","UL-SCH","RACH","Unknown" };

	typedef enum enum_LTENR_PhysicalChannel
	{
		LTENR_PHYSICALCHANNEL_NONE,
		LTENR_PHYSICALCHANNEL_PBCH,		// Physical broadcast channel
		LTENR_PHYSICALCHANNEL_PDCCH,	// Physical downlink control channel
		LTENR_PHYSICALCHANNEL_PDSCH,	// Physical downlink shared channel
		LTENR_PHYSICALCHANNEL_PRACH,	// Physical random access channel
		LTENR_PHYSICALCHANNEL_PUSCH,	// physical uplink shared channel
		LTENR_PHYSICALCHANNEL_PUCCH,	// Physical uplink control channel
		LTENR_PHYSICALCHANNEL_LAST,		// Keep me at last
	}LTENR_PHYSICALCHANNEL;
	static const char* strLTENR_PHYSICALCHANNEL[] =
	{ "NONE","PBCH","PDCCH","PDSCH","PRACH","PUSCH","PUCCH","Unknown" };

	typedef enum enum_LTENR_RLCMode
	{
		LTENR_RLCMODE_TM, // Transparent Mode
		LTENR_RLCMODE_UM, // Unacknowledged mode
		LTENR_RLCMODE_AM, // Acknowledged mode
	}LTENR_RLCMODE;
	static const char* strLTENR_RLCMODE[] =
	{ "Transparent Mode","Unacknowledged Mode", "Acknowledged Mode" };

	typedef enum enum_LTE_NR_SUBLAYER
	{
		LTENR_SUBLAYER_EPC,
		LTENR_SUBLAYER_NAS,
		LTENR_SUBLAYER_SDAP,
		LTENR_SUBLAYER_RRC,
		LTENR_SUBLAYER_PDCP,
		LTENR_SUBLAYER_RLC,
		LTENR_SUBLAYER_MACSCHEDULER,
		LTENR_SUBLAYER_MAC,
		LTENR_SUBLAYER_PHY,
		LTENR_SUBLAYER_LAST, // Keep me at last
	}LTENR_SUBLAYER;

	//EPC
	void LTENR_EPC_NetworkOut();

	//NAS
	void fn_NetSim_NAS_IN();

	//PDCP 
	void fn_NetSim_LTENR_PDCP_TransmitPDCPSDU();
	void fn_NetSim_LTENR_PDCP_ReceivePDCPSDU();
	//RRC
	void fn_NetSim_LTENR_RRC_MacOut();
	void fn_NetSim_LTENR_RRC_MacIn();
	//RLC
	void fn_NetSim_LTENR_RLC_HandlePDUFromUpperLayer();
	void fn_NetSim_LTENR_RLC_HandleMACIN();

	//SDAP
	void fn_NetSim_LTENR_SDAP_HandleMacOut();
	void fn_NetSim_LTENR_SDAP_HandleMacIn();

	//MAC
	void macOut();
	void macIn();

#ifdef OUT
#undef OUT
#define OUT 0
#endif
#ifdef IN
#undef IN
#define IN 1
#endif
	typedef struct stru_LTENR_SublayerInfo
	{
		LTENR_SUBLAYER subLayer;
		char name[50];
		void(*fnOut)(); // Call back function for out event
		void(*fnIn)(); // Call back function for in event
	}LTENR_SUBLAYERINFO, *ptrLTENR_SUBLAYERINFO;
	static LTENR_SUBLAYERINFO subLayerInfo[LTENR_SUBLAYER_LAST] =
	{
		{LTENR_SUBLAYER_EPC,"EPC",LTENR_EPC_NetworkOut,NULL},
		{LTENR_SUBLAYER_NAS,"NAS",NULL,fn_NetSim_NAS_IN},
		{LTENR_SUBLAYER_SDAP,"SDAP",fn_NetSim_LTENR_SDAP_HandleMacOut,fn_NetSim_LTENR_SDAP_HandleMacIn},
		{LTENR_SUBLAYER_RRC,"RRC",fn_NetSim_LTENR_RRC_MacOut, fn_NetSim_LTENR_RRC_MacIn},
		{LTENR_SUBLAYER_PDCP,"PDCP",fn_NetSim_LTENR_PDCP_TransmitPDCPSDU,fn_NetSim_LTENR_PDCP_ReceivePDCPSDU},
		{LTENR_SUBLAYER_RLC,"RLC",fn_NetSim_LTENR_RLC_HandlePDUFromUpperLayer,fn_NetSim_LTENR_RLC_HandleMACIN},
		{LTENR_SUBLAYER_MACSCHEDULER,"MACSCHEDULER",NULL,NULL},
		{LTENR_SUBLAYER_MAC,"MAC",macOut,macIn},
		{LTENR_SUBLAYER_PHY,"PHY",NULL,NULL},
	};
	void fnCallSublayer(LTENR_SUBLAYER layer, int c);
#define LTENR_CallEPCOut()			(fnCallSublayer(LTENR_SUBLAYER_EPC,OUT))
#define LTENR_CallNASOut()			(fnCallSublayer(LTENR_SUBLAYER_NAS,OUT))
#define LTENR_CallNASIn()			(fnCallSublayer(LTENR_SUBLAYER_NAS,IN))
#define LTENR_CallSDAPOut()			(fnCallSublayer(LTENR_SUBLAYER_SDAP,OUT))
#define LTENR_CallSDAPIn()			(fnCallSublayer(LTENR_SUBLAYER_SDAP,IN))
#define LTENR_CallRRCOut()			(fnCallSublayer(LTENR_SUBLAYER_RRC,OUT))
#define LTENR_CallRRCIn()			(fnCallSublayer(LTENR_SUBLAYER_RRC,IN))
#define LTENR_CallPDCPOut()			(fnCallSublayer(LTENR_SUBLAYER_PDCP,OUT))
#define LTENR_CallPDCPIN()			(fnCallSublayer(LTENR_SUBLAYER_PDCP,IN))
#define LTENR_CallRLCOut()			(fnCallSublayer(LTENR_SUBLAYER_RLC,OUT))
#define LTENR_CallRLCIn()			(fnCallSublayer(LTENR_SUBLAYER_RLC,IN))
#define LTENR_CallMACSCHEDULEROut()	(fnCallSublayer(LTENR_SUBLAYER_MACSCHEDULER,OUT))
#define LTENR_CallMACSCHEDULERIn()	(fnCallSublayer(LTENR_SUBLAYER_MACSCHEDULER,IN))
#define LTENR_CallMACOut()			(fnCallSublayer(LTENR_SUBLAYER_MAC,OUT))
#define LTENR_CallMACIn()			(fnCallSublayer(LTENR_SUBLAYER_MAC,IN))

	typedef struct stru_LTENR_ASSOCIATIONINFO
	{
		NETSIM_ID d;
		NETSIM_ID in;
		struct stru_LTENR_ASSOCIATIONINFO* next;
	}LTENR_ASSOCIATIONINFO, *ptrLTENR_ASSOCIATIONINFO;
	void LTENR_ASSOCIATEINFO_ADD(NETSIM_ID d,
								 NETSIM_ID in,
								 NETSIM_ID rd,
								 NETSIM_ID rin);
	void LTENR_ASSOCIATEINFO_REMOVE(NETSIM_ID d,
									NETSIM_ID in,
									NETSIM_ID rd,
									NETSIM_ID rin);
	ptrLTENR_ASSOCIATIONINFO LTENR_ASSOCIATEINFO_FIND(NETSIM_ID d,
													  NETSIM_ID in,
													  NETSIM_ID rd /*OPTIONAL, Return first association*/,
													  NETSIM_ID rin /*OPTIONAL, Return first association for rd*/);
#define LTENR_ISASSOCIATED(d,in,rd,rin) (LTENR_ASSOCIATEINFO_FIND(d,in,rd,rin)!=NULL)
#define LTENR_FIND_ASSOCIATEINTERFACE(d,i,rd) (LTENR_ISASSOCIATED(d,i,rd,0)?LTENR_ASSOCIATEINFO_FIND(d,i,rd,0)->in:0)
#define LTENR_FIND_ASSOCIATEDGNB(ueId,ueIn) (isUE(ueId,ueIn)?LTENR_ASSOCIATEINFO_FIND(ueId,ueIn,0,0)->d:0)
	void fn_NetSim_LTENR_InitAssociation();
	void fn_NetSim_LTENR_RegisterCallBackForAssociation(void(*fnCallBack)(NETSIM_ID gnbId,
																		  NETSIM_ID gnbIf,
																		  NETSIM_ID ueId,
																		  NETSIM_ID ueIf,
																		  bool isAssociated));

	typedef struct stru_LTE_NR_Protocol_Data
	{
		LTENR_DEVICETYPE deviceType;
		NETSIM_ID deviceId;
		NETSIM_ID interfaceId;
		MAC_LAYER_PROTOCOL macProtocol;
		
		//Config parameter -- RRC
		double T300;
		double ueMeasurementReportPeriod;
		double mibPeriod;
		double sib1Period;

		//Config parameter -- PDCP
		double discardTimer;
		bool isHdrCompression;
		bool isoutOfOrderDelivery;
		double t_reorderingTimer;

		//Config parameter -- RLC
		char* maxRetxThreshold;
		char* pollPdu;
		char* pollBytes;
		char* t_reassembly;
		char* t_pollRetransmit;
		char* t_statusProhibit;

		//Config parameter -- MAC
		char* schedulingType;

		//Config paramater -- PHY
		bool antennaValidity;
		bool fastFadingValidity;
		bool blockageValidity;

		ptrLTENR_ASSOCIATIONINFO associationInfo;
		void*** LTENR_LAYER_DATA;

		int series3GPP;
	}LTENR_PROTODATA,*ptrLTENR_PROTODATA;
	ptrLTENR_PROTODATA LTENR_PROTOCOLDATA_ALLOC(NETSIM_ID d,
												NETSIM_ID in);
	void LTENR_PROTOCOLDATA_FREE(NETSIM_ID d,
								 NETSIM_ID in);
	void* LTENR_SUBLAYER_DATA_GET(NETSIM_ID d, NETSIM_ID in,
								  LTENR_DEVICETYPE devType,
								  LTENR_SUBLAYER subLayer);
	void LTENR_SUBLAYER_DATA_SET(NETSIM_ID d, NETSIM_ID in,
								 LTENR_DEVICETYPE devType,
								 LTENR_SUBLAYER subLayer,
								 void* data);
	bool LTENR_SUBLAYER_DATA_IsInitialized(NETSIM_ID d, NETSIM_ID in,
		LTENR_DEVICETYPE devType,
		LTENR_SUBLAYER subLayer);
	ptrLTENR_PROTODATA LTENR_PROTODATA_GET(NETSIM_ID d, NETSIM_ID in);
#define LTENR_UEPHY_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PHY))
#define LTENR_UEMAC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MAC))
#define LTENR_UERLC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RLC))
#define LTENR_UEMACSCHEDULAR_GET(d,i)	(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MACSCHEDULER))
#define LTENR_UEPDCP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PDCP))
#define LTENR_UERRC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RRC))
#define LTENR_UESDAP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_SDAP))
#define LTENR_GNBPHY_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PHY))
#define LTENR_GNBMAC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MAC))
#define LTENR_GNBRLC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RLC))
#define LTENR_GNBMACSCHEDULAR_GET(d,i)	(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MACSCHEDULER))
#define LTENR_GNBPDCP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PDCP))
#define LTENR_GNBRRC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RRC))
#define LTENR_GNBSDAP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_SDAP))
#define LTENR_EPC_GET(d,i)				(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_EPC,LTENR_SUBLAYER_EPC))
#define LTENR_UEPHY_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PHY,data))
#define LTENR_UEMAC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MAC,data))
#define LTENR_UERLC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RLC,data))
#define LTENR_UEMACSCHEDULAR_SET(d,i,data)	(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MACSCHEDULER,data))
#define LTENR_UEPDCP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PDCP,data))
#define LTENR_UERRC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RRC,data))
#define LTENR_UESDAP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_SDAP,data))
#define LTENR_GNBPHY_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PHY,data))
#define LTENR_GNBMAC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MAC,data))
#define LTENR_GNBRLC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RLC,data))
#define LTENR_GNBMACSCHEDULAR_SET(d,i,data)	(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MACSCHEDULER,data))
#define LTENR_GNBPDCP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PDCP,data))
#define LTENR_GNBRRC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RRC,data))
#define LTENR_GNBSDAP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_SDAP,data))
#define LTENR_EPC_SET(d,i,data)				(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_EPC,LTENR_SUBLAYER_EPC,data))
#define LTENR_DEVICETYPE_GET(d,i)			(LTENR_PROTODATA_GET(d,i)->deviceType)
#define LTENR_DEVICETYPE_SET(d,i,type)		(LTENR_PROTODATA_GET(d,i)->deviceType=type)
#define LTENR_GNBPDCP_ISINIT(d,i)			(LTENR_SUBLAYER_DATA_IsInitialized(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PDCP))
#define LTENR_UEPDCP_ISINIT(d,i)			(LTENR_SUBLAYER_DATA_IsInitialized(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PDCP))
#define isUE(d,i)	(LTENR_DEVICETYPE_GET(d,i) == LTENR_DEVICETYPE_UE)
#define isGNB(d,i)	(LTENR_DEVICETYPE_GET(d,i) == LTENR_DEVICETYPE_GNB)
#define isEPC(d,i)	(LTENR_DEVICETYPE_GET(d,i) == LTENR_DEVICETYPE_EPC)
#define setUE(d,i)	(LTENR_DEVICETYPE_SET(d,i,LTENR_DEVICETYPE_UE))
#define setGNB(d,i)	(LTENR_DEVICETYPE_SET(d,i,LTENR_DEVICETYPE_GNB))
#define setEPC(d,i)	(LTENR_DEVICETYPE_SET(d,i,LTENR_DEVICETYPE_EPC))
#define LTENR_PDCP_GET(d,i) (isUE(d,i)?LTENR_UEPDCP_GET(d,i):(isGNB(d,i)?LTENR_GNBPDCP_GET(d,i):NULL))
#define LTENR_RLC_GET(d,i)	(isUE(d,i)?LTENR_UERLC_GET(d,i):(isGNB(d,i)?LTENR_GNBRLC_GET(d,i):NULL))
#define LTENR_SDAP_GET(d,i)	(isUE(d,i)?LTENR_UESDAP_GET(d,i):(isGNB(d,i)?LTENR_GNBSDAP_GET(d,i):NULL))
	

	//LTE_NR Helper 
	bool isLTE_NRInterface(NETSIM_ID d, NETSIM_ID i);
	bool LTENR_GET_PACKET_FROM_ACCESS_BUFFER();
	
	//LTE-NR EPC interface function
	void LTENR_EPC_ASSOCIATION(NETSIM_ID epcId, NETSIM_ID epcIf,
							   NETSIM_ID gnbId, NETSIM_ID gnbIf,
							   NETSIM_ID ueId, NETSIM_ID ueIf);
	void LTENR_EPC_ALLOC(NETSIM_ID d, NETSIM_ID in);
	void LTENR_GNB_SETEPC(NETSIM_ID d, NETSIM_ID in,
						  NETSIM_ID* e, NETSIM_ID* ein);

	//LTE-NR NAS 
	void fn_NetSim_LTENR_NAS_Init();

	//LTE-NR SDAP interface function
	void fn_NetSim_LTENR_SDAP_Init();
	void fn_NetSim_LTENR_UESDAP_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void fn_NetSim_LTENR_GNBSDAP_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	int fn_NetSim_LTENR_SDAP_Metrics_F(PMETRICSWRITER metricsWriter);

	// LTE-NR RRC interface function
	void fn_NetSim_LTENR_RRC_Init();
	void fn_NetSim_LTENR_UERRC_Init(NETSIM_ID ueId,
									NETSIM_ID ueInterface);
	void fn_NetSim_LTENR_GNBRRC_Init(NETSIM_ID gnbId,
									 NETSIM_ID gnbInterface);
	bool fn_NetSim_LTENR_RRC_isConnected(NETSIM_ID d, NETSIM_ID in,
										 NETSIM_ID r, NETSIM_ID rin);

	//LTE-NR PDCP interface function
	void fn_NetSim_LTENR_PDCP_Init();
	void fn_NetSim_LTENR_UEPDCP_Init(NETSIM_ID ueId,
									 NETSIM_ID ueIf);
	void fn_NetSim_LTENR_GNBPDCP_Init(NETSIM_ID gnbId,
									  NETSIM_ID gnbIf);
	
	//LTE-NR RLC interface function
	void fn_NetSim_LTENR_RLC_Init();
	void fn_NetSim_LTENR_UERLC_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void fn_NetSim_LTENR_GNBRLC_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	UINT fn_NetSim_LTENR_RLC_BufferStatusNotificaton(NETSIM_ID d, NETSIM_ID in,
													 NETSIM_ID r, NETSIM_ID rin,
													 LTENR_LOGICALCHANNEL logicalChannel);
	void fn_NetSim_LTENR_RLC_TransmissionStatusNotification(NETSIM_ID d, NETSIM_ID in,
															NETSIM_ID r, NETSIM_ID rin,
															UINT size, LTENR_LOGICALCHANNEL logicalChannel);

	//LTE-NR MAC interface function
	void fn_NetSim_LTENR_MAC_Init();
	void fn_NetSim_LTENR_GNBMAC_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	void fn_NetSim_LTENR_UEMAC_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void LTENR_NotifyMACForStartingSlot();

	//LTE-NR PHY interface function
	void fn_NetSim_LTENR_GNBPHY_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	void fn_NetSim_LTENR_UEPHY_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void fn_NetSim_LTENR_PHY_Init();
	double LTENR_PHY_GetSlotEndTime(NETSIM_ID d, NETSIM_ID in);

	//LTE-NR MSG
	void LTENR_MSG_WriteTrace(NetSim_PACKET* packet, char* s);

	//LTE-NR log
	void fn_NetSim_LTE_MAC_PRBLOG();
	void print_ltenr_PRB_log(char* format, ...);
	bool get_ltenr_PRB_log_status();
	bool get_ltenr_log_status();
	void print_ltenr_log(char* format, ...);
	void print_ltenr_packet_log(NETSIM_ID d, NETSIM_ID in,
								LTENR_SUBLAYER layer, NetSim_PACKET* packet,
								bool isOut);

	//LTE-NR Function def
	void fn_NetSim_LTENR_EPC_RouteHOPacket();
	NETSIM_ID fn_NetSim_Get_LTENR_INTERFACE_ID_FROM_DEVICE_ID(NETSIM_ID r, LTENR_DEVICETYPE type);
	NETSIM_ID fn_NetSim_OTHER_INTERFACE_ID(NETSIM_ID r, NETSIM_ID in);
	void fn_NetSim_PHY_MOBILITY_HANDLE(NETSIM_ID d);

#define LTENR_SDAP_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_SDAP,pstruEventDetails->pPacket,true)
#define LTENR_SDAP_PRINT_PACKET_LOG_IN()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_SDAP,pstruEventDetails->pPacket,false)
#define LTENR_RRC_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RRC,pstruEventDetails->pPacket,true)
#define LTENR_RRC_PRINT_PACKET_LOG_IN()		print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RRC,pstruEventDetails->pPacket,false)
#define LTENR_PDCP_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_PDCP,pstruEventDetails->pPacket,true)
#define LTENR_PDCP_PRINT_PACKET_LOG_IN()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_PDCP,pstruEventDetails->pPacket,false)
#define LTENR_RLC_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RLC,pstruEventDetails->pPacket,true)
#define LTENR_RLC_PRINT_PACKET_LOG_IN()		print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RLC,pstruEventDetails->pPacket,false)
#define LTENR_MAC_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_MAC,pstruEventDetails->pPacket,true)
#define LTENR_MAC_PRINT_PACKET_LOG_IN()		print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_MAC,pstruEventDetails->pPacket,false)

	//CA
#define MAX_CA_COUNT 16

	//LTENR-Utility
	double str_to_time(char* time);
	UINT str_to_bytes(char* bytes);
#define CompareVal(arg0, arg1, op) \
( \
	(!_stricmp(op, ">"))	?	(arg0 > arg1)	:( \
	(!_stricmp(op, ">="))	?	(arg0 >= arg1)	:(\
	(!_stricmp(op, "<"))	?	(arg0 < arg1)	:(\
	(!_stricmp(op, "<="))	?	(arg0 <= arg1)	:(\
	(!_stricmp(op, "=="))	?	(arg0 == arg1)	:(\
	(!_stricmp(op, "!="))	?	(arg0 != arg1)	:false \
))))))

	//debugger function. use when necessary.
	bool ltenr_validate_list(void* ls, UINT size, UINT next, char* file, UINT line);

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTE_NR_H_ */
