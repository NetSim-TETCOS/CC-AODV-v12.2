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
#ifndef _NETSIM_LTENR_RLCMSG_H_
#define _NETSIM_LTENR_RLCMSG_H_

#ifdef  __cplusplus
extern "C" {
#endif

	//SN Size
#define RLC_AM_SN_SIZE	12	//bits
//#define RLC_AM_SN_SIZE	18	//bits
#define RLC_UM_SN_SIZE	6	//bits
//#define RLC_UM_SN_SIZE	12	//bits

#define MIN_RLC_PDU_SIZE 10 //bytes

	//HDR ID
	UINT rlcMsgTMDHDRID;
	UINT rlcMsgAMDHDRID;
	UINT rlcMsgUMDHDRID;
	UINT rlcMsgSTATUSPDUHDRID;

	typedef enum
	{
		SI_ALL,
		SI_FIRST,
		SI_LAST,
		SI_MIDDLE,
	}SI;
	static char strSI[][50] = { "ALL","FIRST","LAST","MIDDLE" };

	typedef struct stru_LTENR_RLCMsg_UMD_Hdr
	{
		SI si;
		UINT R : 6;
		UINT SN;
		UINT16 SO;
	}LTENR_RLCMSG_UMDHDR, *ptrLTENR_RLCMSG_UMDHDR;
#define LTENR_RLCMSG_UMDHDR_SIZE_COMPLETE	1 //Bytes
#if RLC_UM_SN_SIZE == 6
#define LTENR_RLCMSG_UMDHDR_SIZE_NOSO		1 //Bytes
#define LTENR_RLCMSG_UMDHDR_SIZE_WITHSO		3 //Bytes
#elif RLC_UM_SN_SIZE == 12
#define LTENR_RLCMSG_UMDHDR_SIZE_NOSO		2 //Bytes
#define LTENR_RLCMSG_UMDHDR_SIZE_WITHSO		4 //Bytes
#endif
#define LTENR_RLCMSG_UMDHDR_SIZE(si) (((si) == SI_ALL)?LTENR_RLCMSG_UMDHDR_SIZE_COMPLETE:((si)==SI_FIRST?LTENR_RLCMSG_UMDHDR_SIZE_NOSO:LTENR_RLCMSG_UMDHDR_SIZE_WITHSO))

	typedef enum
	{
		DC_CONTROL,
		DC_DATA,
	}DCBIT;
	typedef struct stru_LTENR_RLC_AMD_Hdr
	{
		DCBIT dcBit;
		bool P;
		SI si;
		UINT SN;
		UINT16 SO;
#if RLC_AM_SN_SIZE == 18
		UINT R : 2;
#endif
	}LTENR_RLCMSG_AMDHDR, *ptrLTENR_RLCMSG_AMDHDR;
#if RLC_AM_SN_SIZE == 12
#define LTENR_RLCMSG_AMDHDR_SIZE_NOSO		2 //Bytes
#define LTENR_RLCMSG_AMDHDR_SIZE_WITHSO		4 //Bytes
#elif RLC_AM_SN_SIZE == 18
#define LTENR_RLCMSG_AMDHDR_SIZE_NOSO		3 //Bytes
#define LTENR_RLCMSG_AMDHDR_SIZE_WITHSO		5 //Bytes
#endif
#define LTENR_RLCMSG_AMDHDR_SIZE(si) (((si) == SI_ALL)?LTENR_RLCMSG_AMDHDR_SIZE_NOSO:LTENR_RLCMSG_AMDHDR_SIZE_WITHSO)

	typedef struct stru_LTENR_RLC_StatusPDU_NACK
	{
		UINT NAck_SN;
		bool E1;
		bool E2;
		bool E3;
#if RLC_AM_SN_SIZE == 12
		UINT R : 1;
#elif RLC_AM_SN_SIZE == 18
		UINT R : 3;
#endif
		UINT16 soStart;
		UINT16 soEnd;
		UINT8 NAckRange;

		struct stru_LTENR_RLC_StatusPDU_NACK* next;
	}LTENR_RLCMSG_STATUSPDU_NACK, *ptrLTENR_RLCMSG_STATUSPDU_NACK;

	typedef struct stru_LTENR_RLCMSG_StatusPDU
	{
		DCBIT dcBit;
		UINT CPT : 4;
		UINT Ack_SN;
		bool E1;
#if RLC_AM_SN_SIZE == 12
		UINT R : 7;
#elif RLC_AM_SN_SIZE == 18
		UINT R : 1;
#endif

		UINT NACKCount;
		ptrLTENR_RLCMSG_STATUSPDU_NACK nackHead;
		ptrLTENR_RLCMSG_STATUSPDU_NACK nackTail;
	}LTENR_RLCMSG_STATUSPDU, *ptrLTENR_RLCMSG_STATUSPDU;
#if RLC_AM_SN_SIZE == 12
#define LTENR_RLCMSG_STATUSPDU_FIX_LEN		3 //Bytes
#define LTENR_RLCMSG_STATUSPDU_NACKLEN_0	2 //Bytes
#define LTENR_RLCMSG_STATUSPDU_NACKLEN_1	6 //Bytes
#define LTENR_RLCMSG_STATUSPDU_NACKLEN_2	3 //Bytes
#elif RLC_AM_SN_SIZE == 18
#define LTENR_RLCMSG_STATUSPDU_FIX_LEN		3 //Bytes
#define LTENR_RLCMSG_STATUSPDU_NACKLEN_0	3 //Bytes
#define LTENR_RLCMSG_STATUSPDU_NACKLEN_1	7 //Bytes
#define LTENR_RLCMSG_STATUSPDU_NACKLEN_2	4 //Bytes
#endif
#define LTENR_RLCMSG_STATUSPDU_SETE1(status) ((status)->nackTail?((status)->nackTail->E1=true):((status)->E1=true))

	//Function prototype
	void LTENR_RLCMSG_INIT();

	//TMD HDR
	void LTENR_RLCMSG_TMHDR_ADD(NetSim_PACKET* packet);

	//UMD HDR
	ptrLTENR_RLCMSG_UMDHDR LTENR_RLCMSG_UMHDR_GET(NetSim_PACKET* packet);
	UINT LTENR_RLCMSG_UMHDR_ADD(NetSim_PACKET* packet, SI si, UINT sn, UINT16 so);
	void LTENR_RLCMSG_UMDHDR_REMOVE(NetSim_PACKET* packet);

	void LTENR_RLC_SENDPDUTOMAC(NETSIM_ID d, NETSIM_ID in,
								NetSim_PACKET* packet);
	void LTENR_RLCMSG_PRINT(NetSim_PACKET* packet);

	//AMD HDR
	UINT LTENR_RLCMSG_AMHDR_ADD(NetSim_PACKET* packet, SI si, UINT sn, UINT16 so);
	ptrLTENR_RLCMSG_AMDHDR LTENR_RLCMSG_AMHDR_GET(NetSim_PACKET* packet);
	void LTENR_RLCMSG_AMDHDR_REMOVE(NetSim_PACKET* packet);
	void LTENR_RLCMSG_AMDHDR_SETPOLL(NetSim_PACKET* packet);

	//STATUS PDU
	ptrLTENR_RLCMSG_STATUSPDU LTENR_RLCMSG_STATUSPDU_NEW();
	NetSim_PACKET* LTENR_RLCMSG_STATUSPDU_AddToPacket(void* entity, ptrLTENR_RLCMSG_STATUSPDU status, UINT size);
	void LTENR_RLCMAG_STATUSPDU_FREE(ptrLTENR_RLCMSG_STATUSPDU status);
	void LTENR_RLCMSG_STATUSPDU_AddNack(ptrLTENR_RLCMSG_STATUSPDU status, ptrLTENR_RLCMSG_STATUSPDU_NACK nack);
	ptrLTENR_RLCMSG_STATUSPDU LTENR_RLCMSG_STATUSPDU_GET(NetSim_PACKET* packet);
	bool LTENR_RLCAM_STATUSPDU_FindSN(ptrLTENR_RLCMSG_STATUSPDU pdu, UINT sn);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_RLCMSG_H_
