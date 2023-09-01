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
#include "main.h"
#include "SATELLITE.h"
#include "Satellite_MAC.h"
#include "Satellite_PHY.h"

//Function prototype
static void satellite_ut_init(NETSIM_ID d, NETSIM_ID in);
static void satellite_gw_init(NETSIM_ID d, NETSIM_ID in);
static void satellite_init(NETSIM_ID d, NETSIM_ID in);
static void satellite_handle_timer();
void init_satellite_log();
void close_satellite_log();

_declspec(dllexport) int fn_NetSim_SATELLITE_Init()
{
	init_satellite_log();

	NETSIM_ID i;
	NETSIM_ID j;
	for (i = 0; i < NETWORK->nDeviceCount; i++)
	{
		for (j = 0; j < DEVICE(i + 1)->nNumOfInterface; j++)
		{
			if (!isSatelliteInterface(i + 1, j + 1))
				continue;

			switch (SATELLITE_DEVICETYPE_GET(i + 1, j + 1))
			{
				case SATELLITE_DEVICETYPE_SATELLITE:
					satellite_init(i + 1, j + 1);
					break;
				case SATELLITE_DEVICETYPE_SATELLITE_GATEWAY:
					satellite_gw_init(i + 1, j + 1);
					break;
				case SATELLITE_DEVICETYPE_USER_TERMINAL:
					satellite_ut_init(i + 1, j + 1);
					break;
				default:
					fnNetSimError("Unknown device type in function %s\n", __FUNCTION__);
					break;
			}
		}
	}
	return 0;
}

_declspec(dllexport) int fn_NetSim_SATELLITE_Run()
{
	switch (pstruEventDetails->nEventType)
	{
		case MAC_OUT_EVENT:
			satellite_handle_mac_out();
			break;
		case MAC_IN_EVENT:
			satellite_handle_mac_in();
			break;
		case PHYSICAL_OUT_EVENT:
			satellite_handle_phy_out();
			break;
		case PHYSICAL_IN_EVENT:
			satellite_handle_phy_in();
			break;
		case TIMER_EVENT:
			satellite_handle_timer();
			break;
		default:
			fnNetSimError("Unknown event type %d in function %s\n",
						  pstruEventDetails->nEventType, __FUNCTION__);
			break;
	}
	return 0;
}

_declspec(dllexport) char* fn_NetSim_SATELLITE_Trace(NETSIM_ID id)
{
	UINT i = id % (MAC_PROTOCOL_SATELLITE * 100);
	return (char*)strSATELLITE_SUBEVENT[i];
}

_declspec(dllexport) int fn_NetSim_SATELLITE_FreePacket(NetSim_PACKET* packet)
{
	return 0;
}

_declspec(dllexport) int fn_NetSim_SATELLITE_CopyPacket(NetSim_PACKET* destPacket, const NetSim_PACKET* srcPacket)
{
}

_declspec(dllexport) int fn_NetSim_SATELLITE_Metrics(PMETRICSWRITER file)
{
}

_declspec(dllexport) int fn_NetSim_SATELLITE_Configure(void** var)
{
	fn_NetSim_SATELLITE_Configure_F(var);
	return 0;
}

_declspec(dllexport) char* fn_NetSim_SATELLITE_ConfigPacketTrace(void* xmlNetSimNode)
{
	return "";
}

_declspec(dllexport) int fn_NetSim_SATELLITE_Finish()
{
	close_satellite_log();
}

_declspec(dllexport) int fn_NetSim_SATELLITE_WritePacketTrace(NetSim_PACKET* pstruPacket, char** ppszTrace)
{
	return 0;
}

static void satellite_ut_init(NETSIM_ID d, NETSIM_ID in)
{
	satellite_UT_MAC_init(d, in);
	satellite_ut_phy_init(d, in);
}

static void satellite_gw_init(NETSIM_ID d, NETSIM_ID in)
{
	satellite_GW_MAC_init(d, in);
	satellite_gw_phy_init(d, in);
}

static void satellite_init(NETSIM_ID d, NETSIM_ID in)
{
	satellite_mac_init(d, in);
}

static void satellite_handle_timer()
{
	switch (pstruEventDetails->nSubEventType)
	{
		case SUBEVENT_SUPERFRAME_START:
			satellite_superframe_start();
			break;
		case SUBEVENT_FRAME_START:
			satellite_frame_start();
			break;
		default:
			fnNetSimError("Unknown subevnet %d in function %s\n",
						  pstruEventDetails->nSubEventType, __FUNCTION__);
			break;
	}
}
