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
* Author:    Shashi Kant Suman	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_EMULATIONPACKET_H_
#define _NETSIM_EMULATIONPACKET_H_
#ifdef  __cplusplus
extern "C" {
#endif

	struct stru_real_packet
	{
		HANDLE windivertHandle;

		UINT8* realPacket;
		UINT realPacketLen;

		UINT realPacketCount;
		UINT8* eachRealPacketAddr[MAX_PACKET_COUNT];
		UINT eachRealPacketLen[MAX_PACKET_COUNT];

		void* windivertAddr;
		size_t windivertAddrLen;
		void* eachPacketwindivertAddr[MAX_PACKET_COUNT];

		bool isPacketErroed[MAX_PACKET_COUNT];
	};

	struct stru_emulation_packet
	{
		UINT64 packetId;
		UINT appId;

		ptrREALPACKET realPacket;

		UINT pathLen;
		ptrQUEUE* hops;
		UINT currentHop;

		UINT64 captureTime;
		UINT64 reinjectTime;
		struct stru_emulation_packet* next;
	};

#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_EMULATIONPACKET_H_
