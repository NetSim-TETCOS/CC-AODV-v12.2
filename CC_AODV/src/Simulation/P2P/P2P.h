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
#ifndef _NETSIM_P2P_H_
#define _NETSIM_P2P_H_
//For MSVC compiler. For GCC link via Linker command 
#pragma comment(lib,"Metrics.lib")
#pragma comment(lib,"NetworkStack.lib")
#pragma comment(lib,"Mobility.lib")
#pragma comment(lib,"PropagationModel.lib")

#include "List.h"
#include "ErrorModel.h"

#ifdef  __cplusplus
extern "C" {
#endif

	typedef struct stru_p2p_node_mac
	{
		bool isMacBusy;
	}P2P_NODE_MAC, * ptrP2P_NODE_MAC;
#define P2P_MAC(devid,ifid) ((ptrP2P_NODE_MAC)DEVICE_MACVAR(devid,ifid))
#define P2P_MAC_SET_BUSY(d,in) (P2P_MAC(d,in)->isMacBusy = true)
#define P2P_MAC_SET_IDLE(d,in) (P2P_MAC(d,in)->isMacBusy = false)
#define P2P_MAC_IS_BUSY(d,in) (P2P_MAC(d,in)->isMacBusy)

	/***** Default Config Parameter ****/
#define P2P_CONNECTION_MEDIUM_DEFAULT			_strdup("wired")
#define P2P_BANDWIDTH_DEFAULT					20		//MHz
#define P2P_CENTRAL_FREQUENCY_DEFAULT			30		//MHz
#define P2P_TX_POWER_DEFAULT					20000	//in mW
#define P2P_DATA_RATE_DEFAULT					10		//mbps
#define P2P_RECEIVER_SENSITIVITY_DBM_DEFAULT	-101	//dBm
#define P2P_MODULATION_TECHNIQUE_DEFAULT		_strdup("QPSK")
#define P2P_ANTENNA_HEIGHT_DEFAULT				1
#define P2P_ANTENNA_GAIN_DEFAULT				1
#define P2P_D0_DEFAULT							1
#define P2P_PL_D0_DEFAULT						-30

	typedef struct stru_P2P_NODE_PHY
	{
		bool iswireless;
		double dBandwidth;
		double dCenteralFrequency;
		double dTXPower;
		double dReceiverSensitivity;
		PHY_MODULATION modulation;
		double dDataRate;
		double dAntennaHeight;
		double dAntennaGain;
		double d0;
		double pld0;
	}P2P_NODE_PHY, *ptrP2P_NODE_PHY;
#define P2P_PHY(devid,ifid) ((ptrP2P_NODE_PHY)DEVICE_PHYVAR(devid,ifid))

#define isP2PConfigured(d,i) (DEVICE_MACLAYER(d,i)->nMacProtocolId == MAC_PROTOCOL_P2P)
#define isP2PWireless(d,i) (isP2PConfigured(d,i)?(P2P_PHY(d,i)?P2P_PHY(d,i)->iswireless:false):false)

	PROPAGATION_HANDLE propagationHandle;

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_P2P_H_ */
