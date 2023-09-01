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
#ifndef _NETSIM_SATELLITE_H_
#define _NETSIM_SATELLITE_H_

#include "List.h"

#ifdef  __cplusplus
extern "C" {
#endif

	//For MSVC compiler. For GCC link via Linker command 
#pragma comment(lib,"Metrics.lib")
#pragma comment(lib,"NetworkStack.lib")
#pragma comment(lib,"Mobility.lib")
#pragma comment(lib,"PropagationModel.lib")

	//LOG
#define SATELLITE_LOG
	void print_satellite_log(char* format, ...);
	void satellite_log_add_tab();
	void satellite_log_remove_tab();

// #define SATELLITE_PROPAGATION_LOG

	typedef enum enum_SATELLITE_DEVICE_TYPE
	{
		SATELLITE_DEVICETYPE_USER_TERMINAL,
		SATELLITE_DEVICETYPE_SATELLITE,
		SATELLITE_DEVICETYPE_SATELLITE_GATEWAY,
		SATELLITE_DEVICETYPE_UNKNOWN, //Keep me at last
	}SATELLITE_DEVICETYPE;
	static const char* strSATELLITE_DEVICETYPE[] =
	{ "USER_TERMINAL","SATELLITE","SATELLITE_GATEWAY","Unknown" };
	SATELLITE_DEVICETYPE SATELLITE_DEVICETYPE_FROM_STR(const char* type);

	typedef enum enum_linktype
	{
		LINKTYPE_FORWARD,
		LINKTYPE_RETURN,
	}LINKTYPE;
	static char* strLINKTYPE[] = { "FORWARD","RETURN" };

	typedef enum enum_SATELLITE_LAYER
	{
		SATELLITE_LAYER_MAC,
		SATELLITE_LAYER_PHY,
		SATELLITE_LAYER_UNKNOWN,
	}SATELLITE_LAYER;
	static const char* strSATELLITE_LAYER[] =
	{ "MAC","PHY","Unknwon" };

	typedef enum enum_satellite_subevent
	{
		SUBEVENT_SUPERFRAME_START = MAC_PROTOCOL_SATELLITE * 100 + 1,
		SUBEVENT_FRAME_START,
	}SATELLITE_SUBEVENT;
	static const char* strSATELLITE_SUBEVENT[] =
	{ "","Satellite_Superframe_Start","Satellite_Frame_Start" };

	typedef struct stru_SATELLITE_Protocol_Data
	{
		SATELLITE_DEVICETYPE deviceType;
		NETSIM_ID deviceId;
		NETSIM_ID interfaceId;
		void*** SATELLITE_LAYER_DATA;
	}SATELLITE_PROTODATA, * ptrSATELLITE_PROTODATA;
	ptrSATELLITE_PROTODATA SATELLITE_PROTOCOLDATA_ALLOC(NETSIM_ID d,
														NETSIM_ID in);
#define SATELLITE_PROTOCOLDATA_CURRENT() ((ptrSATELLITE_PROTODATA)DEVICE_MACVAR(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId))
	void SATELLITE_PROTOCOLDATA_FREE(NETSIM_ID d,
									 NETSIM_ID in);
	void* SATELLITE_LAYER_DATA_GET(NETSIM_ID d, NETSIM_ID in,
								   SATELLITE_DEVICETYPE devType,
								   SATELLITE_LAYER layer);
	bool SATELLITE_LAYER_DATA_IsInitialized(NETSIM_ID d, NETSIM_ID in,
											SATELLITE_DEVICETYPE devType,
											SATELLITE_LAYER layer);
	void SATELLITE_LAYER_DATA_SET(NETSIM_ID d, NETSIM_ID in,
								  SATELLITE_DEVICETYPE devType,
								  SATELLITE_LAYER layer,
								  void* data);
	SATELLITE_DEVICETYPE SATELLITE_DEVICETYPE_GET(NETSIM_ID d, NETSIM_ID in);

#define SATELLITE_PHY_GET(d,i)				(SATELLITE_LAYER_DATA_GET(d,i,SATELLITE_DEVICETYPE_SATELLITE,SATELLITE_LAYER_PHY))
#define SATELLITE_MAC_GET(d,i)				(SATELLITE_LAYER_DATA_GET(d,i,SATELLITE_DEVICETYPE_SATELLITE,SATELLITE_LAYER_MAC))
#define SATELLITE_UTPHY_GET(d,i)			(SATELLITE_LAYER_DATA_GET(d,i,SATELLITE_DEVICETYPE_USER_TERMINAL,SATELLITE_LAYER_PHY))
#define SATELLITE_UTMAC_GET(d,i)			(SATELLITE_LAYER_DATA_GET(d,i,SATELLITE_DEVICETYPE_USER_TERMINAL,SATELLITE_LAYER_MAC))
#define SATELLITE_GWPHY_GET(d,i)			(SATELLITE_LAYER_DATA_GET(d,i,SATELLITE_DEVICETYPE_SATELLITE_GATEWAY,SATELLITE_LAYER_PHY))
#define SATELLITE_GWMAC_GET(d,i)			(SATELLITE_LAYER_DATA_GET(d,i,SATELLITE_DEVICETYPE_SATELLITE_GATEWAY,SATELLITE_LAYER_MAC))

#define SATELLITE_PHY_SET(d,i,data)			(SATELLITE_LAYER_DATA_SET(d,i,SATELLITE_DEVICETYPE_SATELLITE,SATELLITE_LAYER_PHY,data))
#define SATELLITE_MAC_SET(d,i,data)			(SATELLITE_LAYER_DATA_SET(d,i,SATELLITE_DEVICETYPE_SATELLITE,SATELLITE_LAYER_MAC,data))
#define SATELLITE_UTPHY_SET(d,i,data)		(SATELLITE_LAYER_DATA_SET(d,i,SATELLITE_DEVICETYPE_USER_TERMINAL,SATELLITE_LAYER_PHY,data))
#define SATELLITE_UTMAC_SET(d,i,data)		(SATELLITE_LAYER_DATA_SET(d,i,SATELLITE_DEVICETYPE_USER_TERMINAL,SATELLITE_LAYER_MAC,data))
#define SATELLITE_GWPHY_SET(d,i,data)		(SATELLITE_LAYER_DATA_SET(d,i,SATELLITE_DEVICETYPE_SATELLITE_GATEWAY,SATELLITE_LAYER_PHY,data))
#define SATELLITE_GWMAC_SET(d,i,data)		(SATELLITE_LAYER_DATA_SET(d,i,SATELLITE_DEVICETYPE_SATELLITE_GATEWAY,SATELLITE_LAYER_MAC,data))

#define isSATELLITE(d,i)					(SATELLITE_DEVICETYPE_GET(d,i) == SATELLITE_DEVICETYPE_SATELLITE)
#define isUT(d,i)							(SATELLITE_DEVICETYPE_GET(d,i) == SATELLITE_DEVICETYPE_USER_TERMINAL)
#define isGW(d,i)							(SATELLITE_DEVICETYPE_GET(d,i) == SATELLITE_DEVICETYPE_SATELLITE_GATEWAY)

	//Function prototype
	//General
	void fn_NetSim_SATELLITE_Configure_F(void**);
	bool isSatelliteInterface(NETSIM_ID d, NETSIM_ID in);

	//Mac
	void satellite_handle_mac_out();
	void satellite_handle_mac_in();

	//PHY
	void satellite_handle_phy_out();
	void satellite_handle_phy_in();

	//HDR
	void satellite_hdr_init(NETSIM_ID d, NETSIM_ID in,
							NetSim_PACKET* packet);

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_SATELLITE_H_ */
