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
#ifndef _NETSIM_IEEE802_11E_H_
#define _NETSIM_IEEE802_11E_H_
#ifdef  __cplusplus
extern "C" {
#endif

#define MAX_AC_CATEGORY	4
#define TU	(1024*MICROSECOND)

	// Data structure for IEEE 802.11e packet lists.
	typedef struct stru_802_11e_Packetlist
	{
		UINT accessCategorty;

		NetSim_PACKET* head;
		NetSim_PACKET* tail;

		double dMaximumSize;
		double dCurrentSize;
	}QOS_LIST,*PQOS_LIST;

	typedef struct stru_edca_config
	{
		UINT acIndex;
		UINT CWmin;
		UINT CWmax;
		UINT AIFSN;
		UINT TXOPLimit;
		UINT MSDULifeTime;

		QOS_LIST qosList;
	}DOT11EDCATABLE, * ptrDOT11EDCATABLE;

	//Function Prototype
	NetSim_PACKET* fn_NetSim_IEEE802_11e_GetPacketFromQueue(NETSIM_ID d, NETSIM_ID in,
															UINT nPacketRequire, UINT* nPacketCount);
	NETSIM_ID fn_NetSim_IEEE802_11e_AddtoQueue(NETSIM_ID d, NETSIM_ID in, NetSim_PACKET* packet);
	bool fn_NetSim_IEEE802_11e_IsPacketInQueue(NETSIM_ID d, NETSIM_ID in);

#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_IEEE802_11E_H_