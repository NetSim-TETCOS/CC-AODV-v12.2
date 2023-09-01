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
#ifndef _NETSIM_LTENR_MSG_H_
#define _NETSIM_LTENR_MSG_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define LTENR_HDRTYPE_MAX 100 //Read-only

	typedef enum enum_LTENR_MSGTYPE
	{

		//RRC
		LTENR_MSG_RRC_MIB = MAC_PROTOCOL_LTE_NR * 100,
		LTENR_MSG_RRC_SIB1,
		LTENR_MSG_RRC_SI,
		LTENR_MSG_RRC_SETUP_REQUEST,
		LTENR_MSG_RRC_SETUP,
		LTENR_MSG_RRC_SETUP_COMPLETE,
		LTENR_MSG_RRC_UE_MEASUREMENT_REPORT,
		LTENR_MSG_RRC_REESTABLISHMENT_REQUEST,
		LTENR_MSG_RRC_REESTABLISHMENT,
		LTENR_MSG_RRC_RECONFIGURATION,
		//NAS
		LTENR_MSG_NAS_HANDOVER_REQUEST,
		LTENR_MSG_NAS_HANDOVER_REQUEST_ACK,
		LTENR_MSG_NAS_HANDOVER_COMMAND,
		LTENR_MSG_NAS_HANDOVER_PATH_SWITCH,
		LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK,
		LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE,
		LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK,
		LTENR_MSG_NAS_HANDOVER_COMPLETE,
		//RLC
		LTENR_MSG_RLC_STATUSPDU,
		LTENR_MSG_RLC_TMDPDU,
		LTENR_MSG_RLC_UMDPDU,
		LTENR_MSG_RLC_AMDPDU,

		LTENR_MSG_USERDATA,
		LTENR_MSG_LAST, //Keep me at last
	}LTENR_MSGTYPE;
	static const char* strLTENR_MSGTYPE[] = { "RRC_MIB","RRC_SIB1","RRC_SI","RRC_SETUP_REQUEST","RRC_SETUP","RRC_SETUP_COMPLETE","UE_MEASUREMENT_REPORT",
		"RRC_REESTABLISHMENT_REQUEST","RRC_REESTABLISHMENT","RRC_RECONFIGURATION","HANDOVER_REQUEST","HANDOVER_REQUEST_ACK","HANDOVER_COMMAND","PATH_SWITCH","PATH_SWICTH_ACK",
		"UE_CONTEXT_RELEASE","UE_CONTEXT_RELEASE_ACK","HANDOVER_COMPLETE","STATUSPDU","TMDPDU","UMDPDU","AMDPDU","UserData","Unknown" };
	

	typedef struct stru_LTENR_HDR
	{
		UINT hdrType;
		UINT size;
		void* hdr;
		struct stru_LTENR_HDR* next;
	}LTENR_HDR, *ptrLTENR_HDR;

	typedef struct stru_LTENR_MSG
	{
		UINT msgType;

		UINT hdrSize;
		ptrLTENR_HDR hdr;
		
		LTENR_LOGICALCHANNEL logicalChannel;
		LTENR_TRANSPORTCHANNEL transportChannel;
		LTENR_PHYSICALCHANNEL physicalChannel;
		LTENR_SRB srb;
		
		LTENR_RLCMODE rlcMode;
	}LTENR_MSG,*ptrLTENR_MSG;
#define LTENR_MSG_GET(packet) ((ptrLTENR_MSG)(packet->pstruMacData->Packet_MACProtocol))
#define LTENR_IS_UPLANE_MSG(packet) ((LTENR_MSG_GET(packet) == NULL) || (LTENR_MSG_GET(packet)->msgType == LTENR_MSG_USERDATA))

#define LTENR_PACKET_GET_LOGICALCHANNEL(packet)				(LTENR_MSG_GET(packet)->logicalChannel)
#define LTENR_PACKET_SET_LOGICALCHANNEL(packet,channel)		(LTENR_MSG_GET(packet)->logicalChannel = channel)
#define LTENR_PACKET_GET_TRANSPORTCHANNEL(packet)			(LTENR_MSG_GET(packet)->transportChannel)
#define LTENR_PACKET_SET_TRANSPORTCHANNEL(packet,channel)	(LTENR_MSG_GET(packet)->transportChannel = channel)
#define LTENR_PACKET_GET_PHYSICALCHANNEL(packet)			(LTENR_MSG_GET(packet)->physicalChannel)
#define LTENR_PACKET_SET_PHYSICALCHANNEL(packet,channel)	(LTENR_MSG_GET(packet)->physicalChannel = channel)

#define LTENR_PACKET_SET_RLCMODE(packet,mode)				(LTENR_MSG_GET(packet)->rlcMode = mode)
#define LTENR_PACKET_GET_RLCMODE(packet)					(LTENR_MSG_GET(packet)->rlcMode)


	ptrLTENR_MSG LTENR_MSG_CREATE(LTENR_MSGTYPE type);
	void LTENR_MSG_FREE(NetSim_PACKET* packet);
	ptrLTENR_MSG LTENR_MSG_COPY(const NetSim_PACKET* packet);
 

	typedef struct stru_LTENR_HDRHANDLER
	{
		UINT hdrType;
		char* hdrName;
		void(*fnHandler)();
		void*(*fnCopyHdr)(void*);
		void(*fnFreeHdr)();
		void(*fnTraceWriter)(void*, char*);
	}LTENR_HDRHANDLER, *ptrLTENR_HDRHANDLER;
	UINT LTENR_HDR_REGISTER(char* name,
							void(*fnHandler)(),
							void*(*fnCopy)(void*),
							void(*fnFree)(void*),
							void(*fnTraceWriter)(void*, char*));
	void LTENR_HDR_ADD_TO_PACKET(NetSim_PACKET* packet,
								 void* hdr,
								 UINT type,
								 UINT size);
	void LTENR_RRC_HDR_ADD_TO_PACKET(NetSim_PACKET* packet,
		void* hdr,
		UINT type,
		UINT size,
		LTENR_MSGTYPE msgType);
	void* LTENR_HDR_GET_FROM_PACKET(NetSim_PACKET* packet,
									UINT type);
	void LTENR_HDR_FREE_FROM_PACKET(NetSim_PACKET* packet,
									UINT type);
	ptrLTENR_HDR LTENR_HDR_NEW(void* hdr,
		UINT type,
		UINT size);

	// LTENR_PACKET
	NetSim_PACKET* LTENR_PACKET_CREATE(NETSIM_ID src, NETSIM_ID dest,
		double time, LTENR_MSGTYPE type);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_MSG_H_
