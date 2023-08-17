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
* Author:    Kumar Gaurav                                                           *
*										                                            *
* ----------------------------------------------------------------------------------*/
#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_GNBRRC.h"
#pragma endregion

#pragma region ENUM

typedef enum enum_LTENR_NAS_HANDOVER_TYPE {
	HANDOVER_TYPE_INTRA_LTENR,
	HANDOVER_TYPE_INTER_LTENR,
	HANDOVER_TYPE_INTER_RAT,
}LTENR_NAS_HANDOVER_TYPE;
static const char* strLTENR_NAS_HANDOVER_TYPE[] = {
		"INTRA_LTENR","INTER_LTENR","INTER_RAT"
};
typedef enum enum_LTENR_NAS_HANDOVER_REQUEST_CAUSE {
	HANDOVER_CAUSE_RADIO_NETWORK,
	HANDOVER_CAUSE_TRANSPORT,
	HANDOVER_CAUSE_NAS,
	HANDOVER_CAUSE_PROTOCOL,
	HANDOVER_CAUSE_MISC,
}LTENR_NAS_HANDOVER_REQUEST_CAUSE;
static const char* strLTENR_NAS_HANDOVER_REQUEST_CAUSE[] = {
		"RADIO_NETWORK","TRANSPORT","NAS","PROTOCOL","MISC"
};
typedef enum enum_LTENR_NAS_HANDOVER_OUTCOME {
	HANDOVER_OUTCOME_SUCESSFUL,
	HANDOVER_OUTCOME_UNSUCESSFUL,
	HANDOVER_OUTCOME_UNAVAILABLE,
	HANDOVER_OUTCOME_UE_LOST,
}LTENR_NAS_HANDOVER_OUTCOME;
static const char* strLTENR_NAS_HANDOVER_OUTCOME[] = {
		"SUCCESSFUL","UNSUCCESSFUL","UNAVAILABLE","UE_LOST"
};
typedef enum enum_LTENR_NAS_CELL_TYPE {
	CELL_TYPE_SMALL,
	CELL_TYPE_MEDIUM,
	CELL_TYPE_LARGE,
}LTENR_NAS_CELL_TYPE;
static const char* strLTENR_NAS_CELL_TYPE[] = {
		"SMALL","MEDIUM","LARGE"
};
typedef enum enum_LTENR_NAS_UECONTEXTRELEASE_STATUS {
	UECONTEXTRELEASE_STATUS_SUCESSFUL,
	UECONTEXTRELEASE_STATUS_UNSUCESSFUL,
}LTENR_NAS_UECONTEXTRELEASE_STATUS;
static const char* strLTENR_NAS_UECONTEXTRELEASE_STATUS[] = {
		"SUCCESSFUL","UNSUCCESSFUL"
};
#pragma endregion

#pragma region STRUCTURE
typedef struct stru_LTENR_NAS_HANDOVER_REQUEST {
	NETSIM_ID MME_UE_ID;//Association Id
	LTENR_NAS_HANDOVER_TYPE type;
	LTENR_NAS_HANDOVER_REQUEST_CAUSE cause;
	NETSIM_ID targetID;
	NETSIM_ID lastVisitedCellID;
	LTENR_NAS_CELL_TYPE LastVisitedCellType;
}LTENR_NAS_HANDOVER_REQUEST, * ptrLTENR_NAS_HANDOVER_REQUEST;

typedef struct stru_LTENR_NAS_HANDOVER_REQUEST_ACK {
	NETSIM_ID MME_UE_ID;
	NETSIM_ID GNB_UE_ID;
	LTENR_NAS_HANDOVER_OUTCOME outCome;
}LTENR_NAS_HANDOVER_REQUEST_ACK, * ptrLTENR_NAS_HANDOVER_REQUEST_ACK;

typedef struct stru_LTENR_NAS_HANDOVER_COMMAND {
	NETSIM_ID MME_UE_ID;//Association Id
	LTENR_NAS_HANDOVER_TYPE type;
	NETSIM_IPAddress E_RADataForwading_DL_TransportLayerAddress;
	//reconfig have to add
}LTENR_NAS_HANDOVER_COMMAND, * ptrLTENR_NAS_HANDOVER_COMMAND;

typedef struct stru_LTENR_NAS_UECONTEXTRELEASEREQUEST {
	NETSIM_ID mme_UE_ID;//Association Id
	NETSIM_ID gnb_UE_ID;
	LTENR_NAS_HANDOVER_REQUEST_CAUSE cause;
}LTENR_NAS_UECONTEXTRELEASEREQUEST, * ptrLTENR_NAS_UECONTEXTRELEASEREQUEST;

typedef struct stru_LTENR_NAS_UECONTEXTRELEASECOMMAND {
	NETSIM_ID ueID;
	LTENR_NAS_HANDOVER_REQUEST_CAUSE cause;
	LTENR_NAS_HANDOVER_OUTCOME outCome;
}LTENR_NAS_UECONTEXTRELEASECOMMAND, * ptrLTENR_NAS_UECONTEXTRELEASECOMMAND;

typedef struct stru_LTENR_NAS_UECONTEXTRELEASECOMPLETE {
	NETSIM_ID mme_UE_ID;
	NETSIM_ID gnb_UE_ID;
	LTENR_NAS_UECONTEXTRELEASE_STATUS status;
}LTENR_NAS_UECONTEXTRELEASECOMPLETE, * ptrLTENR_NAS_UECONTEXTRELEASECOMPLETE;

typedef struct stryu_LTENR_NAS_PATH_SWITCH_REQUEST {
	NETSIM_ID targetGNBID;
	NETSIM_ID associatedEPCtoTarget;
}LTENR_NAS_PATH_SWITCH_REQUEST, * ptrLTENR_NAS_PATH_SWITCH_REQUEST;

typedef struct stryu_LTENR_NAS_PATH_SWITCH_REQUEST_ACK {
	NETSIM_ID targetGNBID;
	NETSIM_ID associatedEPCtoTarget;
}LTENR_NAS_PATH_SWITCH_REQUEST_ACK, * ptrLTENR_NAS_PATH_SWITCH_REQUEST_ACK;

typedef struct stru_LTENR_HANDOVER_Hdr {
	LTENR_MSGTYPE msgType;
	void* msg;
	NETSIM_ID serveringCellID;
	NETSIM_ID serveringCellIF;
	NETSIM_ID targetCellID;
	NETSIM_ID targetCellIF;
	NETSIM_ID UEID;
	NETSIM_ID UEIF;
	NETSIM_ID AMFID;
	NETSIM_ID AMFIF;
}LTENR_HANDOVER_Hdr, * ptrLTENR_HANDOVER_Hdr;

#pragma endregion

#pragma region FUN_DEF

void fn_NetSim_LTENR_NAS_HANDOVER_REQUEST(ptrLTENR_HANDOVER_Hdr msg);
void fn_NetSim_LTENR_NAS_GENERATE_HANDOVER_REQUEST_RECV(ptrLTENR_HANDOVER_Hdr hdr);
void fn_NetSIm_LTENR_NAS_ADD_HDR_INTO_PACKET(NetSim_PACKET* packet, ptrLTENR_HANDOVER_Hdr hdr, UINT hdrID, LTENR_MSGTYPE type);
ptrLTENR_HANDOVER_Hdr LTENR_NAS_HDR_GET(NetSim_PACKET* packet, UINT hdrID);
void LTENR_NAS_HDR_REMOVE(NetSim_PACKET* packet, UINT hdrID);

#pragma endregion
