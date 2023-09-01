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

void satellite_ut_set_gateway(ptrSATELLITE_UT_MAC utMac)
{
	if (!utMac->szGatewayName)
	{
		fnNetSimError("Gateway is not setup for UT %d-%d.\n"
					  "Press any key to terminate simulation.\n",
					  utMac->utId, utMac->utIf);
		int c = _getch();
		exit(c);
	}

	utMac->gatewayId = fn_NetSim_Stack_GetDeviceId_asName(utMac->szGatewayName);
	if (!utMac->gatewayId)
	{
		fnNetSimError("Gateway is not correctly setup for UT %d-%d.\n"
					  "Gateway = %s is not an gateway device\n"
					  "Press any key to terminate simulation.\n",
					  utMac->utId, utMac->utIf, utMac->szGatewayName);
		int c = _getch();
		exit(c);
	}

	utMac->gatewayIf = fn_NetSim_Stack_GetConnectedInterface(utMac->utId, utMac->utIf,
															 utMac->gatewayId);
	if (!utMac->gatewayIf)
	{
		fnNetSimError("%s Gateway is not connected to UT %d-%d.\n"
					  "Press any key to terminate simulation.\n",
					  utMac->szGatewayName, utMac->utId, utMac->utIf);
		int c = _getch();
		exit(c);
	}

	if (DEVICE_MACLAYER(utMac->gatewayId, utMac->gatewayIf)->nMacProtocolId != MAC_PROTOCOL_SATELLITE)
	{
		fnNetSimError("Gateway %s not running satellite protocol.\n"
					  "Press any key to terminate simulation.\n",
					  utMac->szGatewayName);
		int c = _getch();
		exit(c);
	}

	ptrSATELLITE_PROTODATA pd = DEVICE_MACVAR(utMac->gatewayId, utMac->gatewayIf);
	if (pd->deviceType != SATELLITE_DEVICETYPE_SATELLITE_GATEWAY)
	{
		fnNetSimError("Gateway %s is not a satellite gateway device.\n"
					  "Press any key to terminate simulation.\n",
					  utMac->szGatewayName);
		int c = _getch();
		exit(c);
	}
}

bool isSatelliteInterface(NETSIM_ID d, NETSIM_ID in)
{
	if (!DEVICE_INTERFACE(d, in))
		return false;
	if (!DEVICE_INTERFACE(d, in)->pstruMACLayer)
		return false;
	if (DEVICE_MACLAYER(d, in)->nMacProtocolId == MAC_PROTOCOL_SATELLITE)
		return true;
	return false;
}
